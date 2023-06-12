#include <iostream>
#include <conio.h>
#include <filesystem>
#include <fstream>
#include <sec_api/tchar_s.h>
#include "file-watcher.hpp"

namespace fs = std::filesystem;

void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam) {
  static FileSystemWatcher::ACTION pre_act = FileSystemWatcher::ACTION_ERRSTOP;
  switch (action) {
    case FileSystemWatcher::ACTION_ADDED:
      wprintf_s(L"\033[32m添加:\t%s\033[0m\n", _filename);
      break;
    case FileSystemWatcher::ACTION_REMOVED:
      wprintf_s(L"\033[31m删除:\t%s\033[0m\n", _filename);
      break;
    case FileSystemWatcher::ACTION_MODIFIED:
      if (pre_act == FileSystemWatcher::ACTION_MODIFIED) {
        wprintf_s(L"\033[33m修改:\t%s\033[0m\n", _filename);
      }
      break;
    case FileSystemWatcher::ACTION_RENAMED_OLD:
      wprintf_s(L"\033[34m将 [%s] 改名为 ", _filename);
      break;
    case FileSystemWatcher::ACTION_RENAMED_NEW:
      if (pre_act == FileSystemWatcher::ACTION_RENAMED_OLD) {
        wprintf_s(L"[%s]\033[0m\n", _filename);
      }
      break;
    case FileSystemWatcher::ACTION_ERRSTOP:
    default:
      wprintf_s(L"---错误---%s\n", _filename);
      break;
  }
  pre_act = action;
}


void RemoveVersionNumberSuffix(std::string &str) {
  std::size_t dotPosition = str.find_last_of('.');
  if (dotPosition == std::string::npos) { return; }
  std::string suffix = str.substr(dotPosition + 1);
  // 检查后缀是否是数字(如果是)
  if (suffix.find_first_not_of("0123456789") == std::string::npos) {
    str = str.substr(0, dotPosition);
  }
}

bool StartsWith(const std::string &str, const std::string &prefix) {
  if (prefix.length() > str.length())
    return false;
  return std::equal(prefix.begin(), prefix.end(), str.begin());
}

bool EndsWith(const std::string &str, const std::string &suffix) {
  if (suffix.length() > str.length())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

bool IsAllowedFile(const std::string &file_name) {
  bool is_asm = EndsWith(file_name, ".asm");
  bool is_prt = EndsWith(file_name, ".prt");
  return is_asm || is_prt;
}

void TraverseDirectory(const fs::path &directory) {
  {
    // 创建 mnu 文件
    std::ofstream mnu_file(directory / (directory.filename().wstring() + L".mnu"));
    //  mnu_file.imbue(std::locale("chs"));
    mnu_file << directory.filename().string() << "\n#\n#\n";
    // 遍历当前目录下的文件和子目录
    for (const auto &entry: fs::directory_iterator(directory)) {
      if (entry.is_regular_file()) {
        auto file_name = entry.path().filename().string();
        RemoveVersionNumberSuffix(file_name);
        if (IsAllowedFile(file_name)) {
          mnu_file << file_name << '\n';
          mnu_file << std::string("BeiZhu-") << file_name << "\n#\n";
        }
      }
      if (entry.is_directory()) {
        fs::path renamed_path = entry;
        if (!StartsWith(entry.path().filename().string(), "A-")) {
          renamed_path = directory / (L"A-" + entry.path().filename().wstring());
          fs::rename(entry.path(), renamed_path);
        }
        auto folder_name = renamed_path.filename().string();
        mnu_file << "/" << folder_name << '\n';
        mnu_file << std::string("BeiZhu-") << folder_name.substr(2) << "\n#\n";
      }
    }
  }
  // 递归遍历子目录
  for (const auto &entry: fs::directory_iterator(directory)) {
    if (entry.is_directory()) {
      TraverseDirectory(entry.path());
    }
  }
}

int main(int argc, char *argv[]) {
  fs::path directory(argv[1]);
  TraverseDirectory(directory);

  LPCTSTR sDir = argv[1];
  DWORD dwNotifyFilter =
     FileSystemWatcher::FILTER_FILE_NAME
     | FileSystemWatcher::FILTER_DIR_NAME
     | FileSystemWatcher::FILTER_LAST_WRITE_NAME;

  FileSystemWatcher fsw;
  bool r = fsw.Run(sDir, true, dwNotifyFilter, &MyCallback, nullptr);
  if (!r) return -1;

  _tsetlocale(LC_CTYPE, TEXT("chs"));
  wprintf_s(L"监听 [%s] \n", directory.wstring().c_str());
  wprintf_s(L"按 %c 退出.\n", 'q');

  while (_getch() != 'q');
  wprintf_s(L"正在退出...\n");
  fsw.Close(1000);

  return 0;
}
