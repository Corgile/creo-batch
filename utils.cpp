//
// Created by corgi on 2023年6月27日.
//
#include "utils.hpp"

class xhl::dir_item {
public:
    inline std::wstring &short_name() { return this->m_short_name; }

    inline bool &is_file() { return this->m_is_file; }

    inline bool &is_folder() { return this->m_is_folder; }

    inline std::wstring &comment() { return this->m_comment; }

    inline fs::path &absolute_path() { return this->m_absolute_path; }

    inline fs::path &parent_dir() { return this->m_parent_dir; }

public:
    dir_item(std::wstring _short_name, fs::path abs, bool _file)
       : m_short_name(std::move(_short_name)),
         m_absolute_path(std::move(abs)),
         m_is_file(_file), m_is_folder(!_file) {

      m_parent_dir = this->m_absolute_path.parent_path();
      if (m_is_folder) {
        if (not StartsWith(m_short_name, L"A-")) {
          this->m_a_short_name = std::move(L"A-" + m_short_name);
          fs::rename(m_parent_dir / m_short_name, m_parent_dir / m_a_short_name);
        } else {
          this->m_a_short_name = std::move(m_short_name);
        }
        this->m_comment = L"备注-" + m_a_short_name.substr(2);
      }
      if (this->m_is_file) {
        this->m_comment = L"备注-" + this->m_short_name;
      }
    }

    [[nodiscard]] std::wstring to_wstring() const {
      if(m_is_file) {
        return this->m_short_name + L"\n" + this->m_comment + L"\n#\n";
      }
      return L"/" + this->m_a_short_name + L"\n" + this->m_comment + L"\n#\n";
    }

private:
    /// dir_item 的叶子名称(最后一级)
    std::wstring m_short_name;
    /// A-short_name
    std::wstring m_a_short_name;
    /// 当前item是否为文件
    bool m_is_file{false};
    /// 当前item是否为目录
    bool m_is_folder{false};
    /// 备注
    std::wstring m_comment;
    /// 绝对路径
    fs::path m_absolute_path;
    /// 直接父节点,一定是dir
    fs::path m_parent_dir;

};

class xhl::ls_result_holder {
public:
    ls_result_holder()
       : _prt(std::unordered_map<std::wstring, dir_item *>()),
         _asm(std::unordered_map<std::wstring, dir_item *>()) {
    }

    std::unordered_map<std::wstring, dir_item *> _prt;
    std::unordered_map<std::wstring, dir_item *> _asm;
    std::unordered_map<std::wstring, dir_item *> _dir;

    //std::optional<std::reference_wrapper<dir_item>> operator[](std::wstring &trimmed_fn) {
    void insert(dir_item *item) {
      auto trimmed_fn = item->short_name();
      if (this->has(trimmed_fn)) return;

      if (!item->is_file()) {
        this->_dir.insert(std::make_pair(trimmed_fn, item));
        return;
      }

      bool asm_file = EndsWith(trimmed_fn, L".asm");
      bool prt_file = EndsWith(trimmed_fn, L".prt");

      if (asm_file) {
        skip_prt = true;
        if (!this->_prt.empty()) {
          std::for_each(this->_prt.begin(), this->_prt.end(), [&](const auto &item) {
              delete item.second;
          });
          this->_prt.clear();
        }
        this->_asm.insert(std::make_pair(trimmed_fn, item));
      }
      if (!skip_prt && prt_file) {
        this->_prt.insert(std::make_pair(trimmed_fn, item));
      }
    }

    std::unordered_map<std::wstring, dir_item *> items() const {
      if (!this->_dir.empty()) return this->_dir;
      if (!this->_asm.empty()) return this->_asm;
      if (!this->_prt.empty()) return this->_prt;
    }

    ~ ls_result_holder() {

      auto del = [](std::unordered_map<std::wstring, dir_item *> something) {
          std::for_each(something.begin(), something.end(), [&](
             std::pair<std::wstring, dir_item *> item) {
              delete item.second;
              item.second = nullptr;
              something.erase(item.first);
          });
          something.clear();
      };
      del(this->_asm);
      del(this->_prt);
      del(this->_dir);
    }

private:
    auto end() {
      return this->_prt.end();
    }

    bool has(std::wstring &key) {
      bool b1 = this->_prt.find(key) != this->_prt.end();
      bool b2 = this->_asm.find(key) != this->_asm.end();
      bool b3 = this->_dir.find(key) != this->_dir.end();
      return b1 || b2 || b3;
    }

    bool skip_prt{false};
};

#ifdef AUTO_DETECT
void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam) {
      SetConsoleOutputCP(65001);
      fs::path modified_item(_filename);
      if (modified_item.filename().string().find(".swp") != std::string::npos) { return; }
      if (modified_item.filename() == "GBLib.ctg.1") { return; }

      auto abs_path_of_modified_item = xhl::gb_lib_home / modified_item;

      static FileSystemWatcher::ACTION pre_act = FileSystemWatcher::ACTION_ERRSTOP;
      switch (action) {
        case FileSystemWatcher::ACTION_ADDED:
          if (fs::is_regular_file(abs_path_of_modified_item)) {
            auto f = modified_item.filename().wstring();
            TrimNumSuffix(f);
            if (!IsAllowedFile(f, false)) { break; }
            std::printf("\033[32m++ 添加文件 [%s]\033[0m\n", modified_item.string().c_str());
            GenerateMenuFileInCurrentDir(abs_path_of_modified_item.parent_path());
          }
          if (fs::is_directory(abs_path_of_modified_item)) {
            std::printf("\033[32m++ 添加文件夹 [%s]\033[0m\n", modified_item.string().c_str());
            auto last_dir = abs_path_of_modified_item.filename();
            if (fs::exists(fs::path(L"A-" + last_dir.wstring()))) { break; }
            GenerateMenuFileInCurrentDir(abs_path_of_modified_item.parent_path());
            if (last_dir.wstring().find(L"A-") != std::wstring::npos) {
              GenerateMenuFileInCurrentDir(xhl::gb_lib_home / modified_item.parent_path() / last_dir);
            } else {
              GenerateMenuFileInCurrentDir(
                 xhl::gb_lib_home / modified_item.parent_path() / fs::path(L"A-" + last_dir.wstring()));
            }
          }
          break;
        case FileSystemWatcher::ACTION_REMOVED:
          std::printf("\033[31mX 删除 [%s]\033[0m\n", modified_item.string().c_str());
          if (fs::is_regular_file(abs_path_of_modified_item)) {
            auto f = modified_item.filename().wstring();
            TrimNumSuffix(f);
            if (!IsAllowedFile(f, false)) { break; }
          }
          if (fs::exists(abs_path_of_modified_item.parent_path())) {
            GenerateMenuFileInCurrentDir(abs_path_of_modified_item.parent_path());
          }
          break;
        case FileSystemWatcher::ACTION_RENAMED_OLD:
          std::printf("\033[33m<> 将 [%s] 重命名为 ", modified_item.string().c_str());
          //      old_name_of_mnu = fs::path(modified_item.m_short_name().wstring().append(L".mnu"));
          break;
        case FileSystemWatcher::ACTION_RENAMED_NEW:
          if (pre_act != FileSystemWatcher::ACTION_RENAMED_OLD) { break; }
          if (fs::is_regular_file(abs_path_of_modified_item)) {
            std::printf("[%s]\033[0m\n", modified_item.string().c_str());
            GenerateMenuFileInCurrentDir(abs_path_of_modified_item.parent_path());
          }
          if (fs::is_directory(abs_path_of_modified_item)) {
            std::printf("[%s]\033[0m\n", modified_item.string().c_str());
            GenerateMenuFileInCurrentDir(abs_path_of_modified_item.parent_path());
            if (modified_item.filename().wstring().find(L"A-") == std::wstring::npos) {
              auto r = fs::path(L"A-" + modified_item.filename().wstring());
              GenerateMenuFileInCurrentDir(xhl::gb_lib_home / modified_item.parent_path() / r);
            } else {
              GenerateMenuFileInCurrentDir(abs_path_of_modified_item);
            }
          }
          break;
        case FileSystemWatcher::ACTION_MODIFIED:
          std::printf("\t\033[35m * 修改 [%s] \033[0m\n", modified_item.string().c_str());
          break;
        case FileSystemWatcher::ACTION_ERRSTOP:
        default:
          break;
      }
      pre_act = action;
      system("%PRO_LIBRARY_DIR%\\pro_build_library_ctg.exe > nul");
    }
#endif

std::wstring xhl::TrimNumSuffix(std::wstring str) {
  std::size_t dotPosition = str.find_last_of('.');
  if (dotPosition == std::wstring::npos) { return str; }
  std::wstring suffix = str.substr(dotPosition + 1);// 检查后缀是否是数字(如果是)
  if (suffix.find_first_not_of(L"0123456789") == std::wstring::npos) {
    str = str.substr(0, dotPosition);
  }
  return str;
}

bool xhl::StartsWith(const std::wstring &str, const std::wstring &prefix) {
  if (prefix.length() > str.length())
    return false;
  return std::equal(prefix.begin(), prefix.end(), str.begin());
}

bool xhl::EndsWith(const std::wstring &str, const std::wstring &suffix) {
  if (suffix.length() > str.length())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

bool xhl::IsAllowedFile(const std::wstring &file_name, bool hasAsm) {
  bool is_asm = EndsWith(file_name, L".asm");
  if (hasAsm) return is_asm;
  return is_asm || EndsWith(file_name, L".prt");
}

void xhl::GenerateMenuFileInCurrentDir(const fs::path &current_dir) {
  bool has_prt{false}, has_asm{false}, has_sub_dir{false};
  xhl::ls_result_holder ls;
  /// pre-check
  for (const auto &entry: fs::directory_iterator(current_dir)) {
    auto filename = TrimNumSuffix(entry.path().filename().wstring());
    if (EndsWith(filename, L".mnu")) {
      fs::remove(entry);
      continue;
    }
    bool is_asm = EndsWith(filename, L".asm");
    bool is_prt = EndsWith(filename, L".prt");
    has_asm |= is_asm;
    has_prt |= is_prt;
    has_sub_dir |= entry.is_directory();
  }
  /// 如果最后一层文件夹中没有asm或prt文件，则不生成mnu文件
  if (!has_sub_dir && !has_asm && !has_prt) { return; }

  for (const auto &entry: fs::directory_iterator(current_dir)) {
    auto filename = TrimNumSuffix(entry.path().filename().wstring());
    bool allowed_file_types = EndsWith(filename, L".asm")||EndsWith(filename, L".prt");

    if (entry.is_regular_file() && !allowed_file_types) {
      continue;
    }

    ls.insert(new dir_item(filename, entry.path(), entry.is_regular_file()));
  }


  auto temp = current_dir.filename().wstring() + L".mnu";

  auto mnu_file_name = current_dir / fs::path(temp);
  // 创建 mnu 文件
  std::ofstream mnu_file(mnu_file_name, std::ios::out | std::ios::binary);
  std::wstringstream mnu_content;
  //auto wtitle = current_dir.filename().wstring() + L"\n#\n#\n";
  mnu_content << temp << L"\n#\n#\n";
  //mnu_file << ToGBK(wtitle);
  for (const auto &item: ls.items()) {
    mnu_content << item.second->to_wstring();
  }
  auto utf16Text = std::move(mnu_content.str());
  mnu_file << ToGBK(utf16Text);
  mnu_file.flush();
  mnu_file.close();
}

std::string xhl::ToGBK(std::wstring &utf16Text) {
  int requiredSize = WideCharToMultiByte(0, 0, utf16Text.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (requiredSize <= 0) {
    std::cerr << "Failed to calculate required buffer size" << std::endl;
    return {};
  }
  std::string gbkText(requiredSize, '\0');
  if (WideCharToMultiByte(0, 0, utf16Text.c_str(), -1, &gbkText[0], requiredSize, nullptr, nullptr) != 0) {
    gbkText.resize(requiredSize - 1);// 去除结尾的 null 字符
    return gbkText;
  }
  std::cerr << "Failed to convert UTF-16 to GBK" << std::endl;
  return {};
}

void xhl::TraverseDirectoryNR(const fs::path &directory) {
  std::queue<xhl::fs::path> queue;
  queue.push(directory);
  while (!queue.empty()) {
    fs::path current = queue.front();
    queue.pop();
    if (fs::is_regular_file(current)) continue;
    GenerateMenuFileInCurrentDir(current);
    for (const auto &entry: fs::directory_iterator(current)) {
      queue.push(entry);
    }
  }
}