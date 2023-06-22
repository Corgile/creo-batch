#include <filesystem>
#include <fstream>
#include <set>
#include <queue>
#include <sstream>
#include <iostream>
#include <conio.h>
#include "file-watcher.hpp"

namespace fs = std::filesystem;

void ProcessCurrentDirectory(const fs::path &current_dir);

void RemoveVersionNumberSuffix(std::wstring &str);

bool IsAllowedFile(const std::wstring &file_name, bool hasAsm);

std::string toGbk(std::wstring &utf16Text);

fs::path cwd_prefix;
FileSystemWatcher *watcher;

void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam) {
  SetConsoleOutputCP(65001);
  fs::path modified_item(_filename);
  if (modified_item.filename().string().find(".swp") != std::string::npos) return;
  if (modified_item.filename() == "GBLib.ctg.1") return;

  auto abs_path_of_modified_item = cwd_prefix / modified_item;

  static FileSystemWatcher::ACTION pre_act = FileSystemWatcher::ACTION_ERRSTOP;
  switch (action) {
    case FileSystemWatcher::ACTION_ADDED:
      if (fs::is_regular_file(abs_path_of_modified_item)) {
        auto f = modified_item.filename().wstring();
        RemoveVersionNumberSuffix(f);
        if (!IsAllowedFile(f, false)) break;
        std::printf("\033[32m++ 添加文件 [%s]\033[0m\n", modified_item.string().c_str());
        ProcessCurrentDirectory(abs_path_of_modified_item.parent_path());
      }
      if (fs::is_directory(abs_path_of_modified_item)) {
        std::printf("\033[32m++ 添加文件夹 [%s]\033[0m\n", modified_item.string().c_str());
        auto last_dir = abs_path_of_modified_item.filename();
        if (fs::exists(fs::path(L"A-" + last_dir.wstring()))) {
          break;
        }
        ProcessCurrentDirectory(abs_path_of_modified_item.parent_path());
        if (last_dir.wstring().find(L"A-") != std::wstring::npos) {
          ProcessCurrentDirectory(cwd_prefix / modified_item.parent_path() / last_dir);
        } else {
          ProcessCurrentDirectory(cwd_prefix / modified_item.parent_path() / fs::path(L"A-" + last_dir.wstring()));
        }
      }
      break;
    case FileSystemWatcher::ACTION_REMOVED:
      std::printf("\033[31mX 删除 [%s]\033[0m\n", modified_item.string().c_str());
      if (fs::is_regular_file(abs_path_of_modified_item)) {
        auto f = modified_item.filename().wstring();
        RemoveVersionNumberSuffix(f);
        if (!IsAllowedFile(f, false)) break;
      }
      if (fs::exists(abs_path_of_modified_item.parent_path()))
        ProcessCurrentDirectory(abs_path_of_modified_item.parent_path());
      break;
    case FileSystemWatcher::ACTION_RENAMED_OLD:
      std::printf("\033[33m<> 将 [%s] 重命名为 ", modified_item.string().c_str());
//      old_name_of_mnu = fs::path(modified_item.filename().wstring().append(L".mnu"));
      break;
    case FileSystemWatcher::ACTION_RENAMED_NEW:
      if (pre_act != FileSystemWatcher::ACTION_RENAMED_OLD) {
        break;
      }
      if (fs::is_regular_file(abs_path_of_modified_item)) {
        std::printf("[%s]\033[0m\n", modified_item.string().c_str());
        ProcessCurrentDirectory(abs_path_of_modified_item.parent_path());
      }
      if (fs::is_directory(abs_path_of_modified_item)) {
        std::printf("[%s]\033[0m\n", modified_item.string().c_str());
        ProcessCurrentDirectory(abs_path_of_modified_item.parent_path());
        if (modified_item.filename().wstring().find(L"A-") == std::wstring::npos) {
          auto r = fs::path(L"A-" + modified_item.filename().wstring());
          ProcessCurrentDirectory(cwd_prefix / modified_item.parent_path() / r);
        } else {
          ProcessCurrentDirectory(abs_path_of_modified_item);
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

void RemoveVersionNumberSuffix(std::wstring &str) {
  std::size_t dotPosition = str.find_last_of('.');
  if (dotPosition == std::wstring::npos) { return; }
  std::wstring suffix = str.substr(dotPosition + 1);
  // 检查后缀是否是数字(如果是)
  if (suffix.find_first_not_of(L"0123456789") == std::wstring::npos) {
    str = str.substr(0, dotPosition);
  }
}

bool StartsWith(const std::wstring &str, const std::wstring &prefix) {
  if (prefix.length() > str.length())
    return false;
  return std::equal(prefix.begin(), prefix.end(), str.begin());
}

bool EndsWith(const std::wstring &str, const std::wstring &suffix) {
  if (suffix.length() > str.length())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

bool IsAllowedFile(const std::wstring &file_name, bool hasAsm) {
  bool is_asm = EndsWith(file_name, L".asm");
  if (hasAsm) return is_asm;
  return is_asm || EndsWith(file_name, L".prt");
}

bool HasAsmFile(const fs::path &current_dir) {
  for (const auto &entry: fs::directory_iterator(current_dir)) {
    if (entry.path().filename().wstring().find(L".asm") != std::wstring::npos) {
      return true;
    }
  }
  return false;
}

void ProcessCurrentDirectory(const fs::path &current_dir) {

  auto mnu_file_name = current_dir / (current_dir.filename().wstring() + L".mnu");
  if (!fs::exists(mnu_file_name)) {
    for (const auto &entry: fs::directory_iterator(current_dir)) {
      if (entry.is_regular_file()) {
        if (entry.path().filename().string().find(".mnu") != std::string::npos) {
          fs::remove(entry);
        }
      }
    }
  }
  // 创建 mnu 文件
  std::ofstream mnu_file(mnu_file_name, std::ios::out);
  std::wstringstream wcontent;
  wcontent << current_dir.filename().wstring() << L"\n#\n#\n";
  bool hasAsm = HasAsmFile(current_dir);
  // 遍历当前目录下的文件和子目录
  for (const auto &entry: fs::directory_iterator(current_dir)) {
    if (entry.is_regular_file()) {
      auto file_name = entry.path().filename().wstring();
      RemoveVersionNumberSuffix(file_name);
      if (IsAllowedFile(file_name, hasAsm)) {
        wcontent << file_name << L"\n备注-" << file_name << L"\n#\n";
      }
    }
    if (entry.is_directory()) {
      fs::path renamed_path = entry;
      if (!StartsWith(entry.path().filename().wstring(), L"A-")) {
        renamed_path = current_dir / (L"A-" + entry.path().filename().wstring());
        if (fs::exists(renamed_path)) {
          auto t = current_dir / entry;
          std::printf("[\033[31m%s\033[0m]已经存在A-开头的文件夹，不再对其进行重命名, 跳过\n", t.string().c_str());
        } else { fs::rename(entry.path(), renamed_path); }
      }
      auto folder_name = renamed_path.filename().wstring();
      wcontent << L"/" << folder_name << L"\n备注-" << folder_name.substr(2) << L"\n#\n";
    }
  }
  std::wstring utf16Text = wcontent.str();
  mnu_file << toGbk(utf16Text);
}

std::string toGbk(std::wstring &utf16Text) {
  int requiredSize = WideCharToMultiByte(CP_ACP, 0, utf16Text.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (requiredSize > 0) {
    std::string gbkText(requiredSize, '\0');
    if (WideCharToMultiByte(CP_ACP, 0, utf16Text.c_str(), -1, &gbkText[0], requiredSize, nullptr, nullptr) != 0) {
      gbkText.resize(requiredSize - 1);  // 去除结尾的 null 字符
      return gbkText;
    } else {
      std::cerr << "Failed to convert UTF-16 to GBK" << std::endl;
    }
  } else {
    std::cerr << "Failed to calculate required buffer size" << std::endl;
  }
  return "";
}

void TraverseDirectoryNR(const fs::path &directory) {
  std::queue<fs::path> queue;
  queue.push(directory);
  while (!queue.empty()) {
    fs::path current = queue.front();
    queue.pop();
    if (fs::is_regular_file(current)) continue;
    ProcessCurrentDirectory(current);
    for (const auto &entry: fs::directory_iterator(current)) {
      queue.push(entry);
    }
  }
}

int main(int argc, char *argv[]) {
  SetConsoleOutputCP(65001);
  const char *proLibraryDir = std::getenv("PRO_LIBRARY_DIR");

  if (!proLibraryDir) {
    std::cout << "环境变量 PRO_LIBRARY_DIR 未设置" << std::endl;
    if (argc == 1) {
      std::cout << ", 程序并且缺少必要路径参数, 正在退出程序.." << std::endl;
      exit(EXIT_FAILURE);
    } else {
      cwd_prefix = fs::path(argv[1]);
      if (!fs::exists(cwd_prefix)) {
        std::wcout << "路径 [" << cwd_prefix.wstring() << "] 不存在, 正在退出程序.." << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  } else {
    cwd_prefix = fs::path(proLibraryDir);
  }

  std::cout << "- 正在生成 .mnu，请稍候" << std::endl;
  TraverseDirectoryNR(cwd_prefix);
  system("%PRO_LIBRARY_DIR%\\pro_build_library_ctg.exe > nul");
  std::cout << "- 创建 .mnu 已完成" << std::endl;
  std::cout << "如需开启自动检测变化功能，按 y/Y. 其他任意键退出程序" << std::endl;

  int keyCode = _getch();
  if (keyCode != 'y' && keyCode != 'Y') {
    SetConsoleOutputCP(936);
    system("pause");
    return 0;
  }

  auto sDir = cwd_prefix.string();
  DWORD dwNotifyFilter =
      FileSystemWatcher::FILTER_FILE_NAME
      | FileSystemWatcher::FILTER_DIR_NAME
      | FileSystemWatcher::FILTER_LAST_ACCESS_NAME;

  watcher = new FileSystemWatcher(sDir.c_str(), true, dwNotifyFilter, &MyCallback, nullptr);
  if (!watcher->start()) return -1;
  std::cout << "- 正在检测 [" << sDir << "] 的变化, 按 q 退出.\n";
  while (_getch() != 'q');
  watcher->stop(1);
  SetConsoleOutputCP(936);

  return 0;
}
