#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-use-anyofallof"
//// Created by corgi on 2023年6月23日.//

#ifndef CMD_SCRIPT_UTILS_HPP
#define CMD_SCRIPT_UTILS_HPP

#include "file-watcher.hpp"
#include <filesystem>
#include <fstream>
#include <set>
#include <queue>
#include <sstream>
#include <iostream>
#include <conio.h>


namespace xhl {
    namespace fs = std::filesystem;
    fs::path cwd_prefix;
    FileSystemWatcher *watcher;

    void GenerateMenuFileInCurrentDir(const fs::path &dir);

    void RemoveVersionNumberSuffix(std::wstring &str);

    bool StartsWith(const std::wstring &str, const std::wstring &prefix);

    bool EndsWith(const std::wstring &str, const std::wstring &suffix);

    bool IsAllowedFile(const std::wstring &file_name, bool hasAsm);

    void CheckSatisfiedDir(const fs::path &current_dir, bool &hasAsm, bool &hasPrt, bool &hasSubDir);

    std::string toGbk(std::wstring &utf16Text);

    void TraverseDirectoryNR(const fs::path &directory);

    void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam);

    void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam) {
      SetConsoleOutputCP(65001);
      fs::path modified_item(_filename);
      if (modified_item.filename().string().find(".swp") != std::string::npos) { return; }
      if (modified_item.filename() == "GBLib.ctg.1") { return; }

      auto abs_path_of_modified_item = cwd_prefix / modified_item;

      static FileSystemWatcher::ACTION pre_act = FileSystemWatcher::ACTION_ERRSTOP;
      switch (action) {
        case FileSystemWatcher::ACTION_ADDED:
          if (fs::is_regular_file(abs_path_of_modified_item)) {
            auto f = modified_item.filename().wstring();
            RemoveVersionNumberSuffix(f);
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
              GenerateMenuFileInCurrentDir(cwd_prefix / modified_item.parent_path() / last_dir);
            } else {
              GenerateMenuFileInCurrentDir(
                 cwd_prefix / modified_item.parent_path() / fs::path(L"A-" + last_dir.wstring()));
            }
          }
          break;
        case FileSystemWatcher::ACTION_REMOVED:
          std::printf("\033[31mX 删除 [%s]\033[0m\n", modified_item.string().c_str());
          if (fs::is_regular_file(abs_path_of_modified_item)) {
            auto f = modified_item.filename().wstring();
            RemoveVersionNumberSuffix(f);
            if (!IsAllowedFile(f, false)) { break; }
          }
          if (fs::exists(abs_path_of_modified_item.parent_path())) {
            GenerateMenuFileInCurrentDir(abs_path_of_modified_item.parent_path());
          }
          break;
        case FileSystemWatcher::ACTION_RENAMED_OLD:
          std::printf("\033[33m<> 将 [%s] 重命名为 ", modified_item.string().c_str());
          //      old_name_of_mnu = fs::path(modified_item.filename().wstring().append(L".mnu"));
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
              GenerateMenuFileInCurrentDir(cwd_prefix / modified_item.parent_path() / r);
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

    void RemoveVersionNumberSuffix(std::wstring &str) {
      std::size_t dotPosition = str.find_last_of('.');
      if (dotPosition == std::wstring::npos) { return; }
      std::wstring suffix = str.substr(dotPosition + 1);// 检查后缀是否是数字(如果是)
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

    void CheckSatisfiedDir(const fs::path &current_dir, bool &hasAsm, bool &hasPrt, bool &hasSubDir) {
      for (const auto &entry: fs::directory_iterator(current_dir)) {
        hasAsm |= (entry.path().filename().wstring().find(L".asm") != std::wstring::npos);
        hasPrt |= (entry.path().filename().wstring().find(L".prt") != std::wstring::npos);
        hasSubDir |= entry.is_directory();
      }
    }

    void GenerateMenuFileInCurrentDir(const fs::path &current_dir) {
      bool has_prt{false}, has_asm{false}, has_sub_dir{false};
      CheckSatisfiedDir(current_dir, has_asm, has_prt, has_sub_dir);
      if (!has_asm && !has_prt && !has_sub_dir) { return; }

      auto mnu_file_name = current_dir / (current_dir.filename().wstring() + L".mnu");
      for (const auto &entry: fs::directory_iterator(current_dir)) {
        if (entry.is_regular_file()) {
          if (EndsWith(entry.path().wstring(), L".mnu")) {
            fs::remove(entry);
          }
        }
      }// 创建 mnu 文件
      std::ofstream mnu_file(mnu_file_name, std::ios::out);
      auto wtitle = current_dir.filename().wstring() + L"\n#\n#\n";
      mnu_file << toGbk(wtitle);
      std::set<std::wstring> item_set;// 遍历当前目录下的文件和子目录
      for (const auto &entry: fs::directory_iterator(current_dir)) {
        std::wstringstream wcontent;
        if (entry.is_regular_file()) {
          auto file_name = entry.path().filename().wstring();
          RemoveVersionNumberSuffix(file_name);
          if (IsAllowedFile(file_name, has_asm)) {
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
        item_set.insert(wcontent.str());
        wcontent.str({});
      }
      for (auto item: item_set) {
        mnu_file << toGbk(item);
      }
      item_set.clear();
    }

    std::string toGbk(std::wstring &utf16Text) {
      int requiredSize = WideCharToMultiByte(CP_ACP, 0, utf16Text.c_str(), -1, nullptr, 0, nullptr, nullptr);
      if (requiredSize <= 0) {
        std::cerr << "Failed to calculate required buffer size" << std::endl;
        return {};
      }
      std::string gbkText(requiredSize, '\0');
      if (WideCharToMultiByte(CP_ACP, 0, utf16Text.c_str(), -1, &gbkText[0], requiredSize, nullptr, nullptr) != 0) {
        gbkText.resize(requiredSize - 1);// 去除结尾的 null 字符
        return gbkText;
      }
      std::cerr << "Failed to convert UTF-16 to GBK" << std::endl;
      return {};
    }

    void TraverseDirectoryNR(const fs::path &directory) {
      std::queue<fs::path> queue;
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

}

#endif//CMD_SCRIPT_UTILS_HPP

#pragma clang diagnostic pop