#include "file-watcher.hpp"
#include <iostream>
#include <cassert>
#include <tchar.h>
#include <conio.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR filename, LPVOID lParam) {
  static FileSystemWatcher::ACTION pre_act = FileSystemWatcher::ACTION_ERRSTOP;
  switch (action) {
    /*case FileSystemWatcher::FILTER_LAST_ACCESS_NAME:
      wprintf_s(L"访问:\t%s\n", filename);
      break;*/
    case FileSystemWatcher::ACTION_ADDED:
      wprintf_s(L"\033[32m添加:\t%s\033[0m\n", filename);
      break;
    case FileSystemWatcher::ACTION_REMOVED:
      wprintf_s(L"\033[31m删除:\t%s\033[0m\n", filename);
      break;
    case FileSystemWatcher::ACTION_MODIFIED:
      if (pre_act == FileSystemWatcher::ACTION_MODIFIED)
        wprintf_s(L"\033[33m修改:\t%s\033[0m\n", filename);
      break;
    case FileSystemWatcher::ACTION_RENAMED_OLD:
      wprintf_s(L"\033[34m将 [%s] 改名为 ", filename);
      break;
    case FileSystemWatcher::ACTION_RENAMED_NEW:
      assert(pre_act == FileSystemWatcher::ACTION_RENAMED_OLD);
      wprintf_s(L"[%s]\033[0m\n", filename);
      break;
    case FileSystemWatcher::ACTION_ERRSTOP:
    default:
      wprintf_s(L"---错误---%s\n", filename);
      break;
  }
  pre_act = action;
}


void RemoveVersionNumberSuffix(std::string &str) {
  std::size_t dotPosition = str.find_last_of('.');
  if (dotPosition == std::string::npos) { return; }
  std::string suffix = str.substr(dotPosition + 1);
  // 检查后缀是否是数字
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

bool IsAllowedFile(std::string &file_name) {
  bool is_asm = EndsWith(file_name, ".asm");
  bool is_prt = EndsWith(file_name, ".prt");
  return is_asm || is_prt;
}

void TraverseDirectory(const fs::path &directory) {
  // 创建 mnu 文件
  std::ofstream mnuFile(directory / (directory.filename().wstring() + L".mnu"));
  mnuFile.imbue(std::locale(""));
  mnuFile << directory.filename().string() << "\n#\n#\n";
  // 遍历当前目录下的文件和子目录
  for (const auto &entry: fs::directory_iterator(directory)) {
    if (entry.is_regular_file()) {
      auto file_name = entry.path().filename().string();
      RemoveVersionNumberSuffix(file_name);
      std::cout << "\033[31m" << file_name << "\033[0m\n";
      if (IsAllowedFile(file_name)) {
        mnuFile << file_name << '\n';
        mnuFile << std::string("BeiZhu-") << file_name << "\n#\n";
      }
    }
    if (entry.is_directory()) {
      fs::path renamedPath = entry;
      if (!StartsWith(entry.path().string(), "A-")) {
        renamedPath = directory / (L"A-" + entry.path().filename().wstring());
        fs::rename(entry.path(), renamedPath);
      }
      auto folder_name = renamedPath.filename().string();
      mnuFile << "/" << folder_name << '\n';
      mnuFile << std::string("BeiZhu-") << folder_name << "\n#\n";
    }
  }
  mnuFile.clear();
  mnuFile.close();
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
  return 0;
}


int _main(int argc, char *argv[]) {
  LPCTSTR sDir = TEXT("C:\\Users\\gzhu03\\IdeaProjects\\cmd-script\\example");
  DWORD dwNotifyFilter =
      FileSystemWatcher::FILTER_FILE_NAME
      | FileSystemWatcher::FILTER_DIR_NAME
      | FileSystemWatcher::FILTER_LAST_WRITE_NAME;

  FileSystemWatcher fsw;
  bool r = fsw.Run(sDir, true, dwNotifyFilter, &MyCallback, nullptr);
  if (!r) return -1;

  _tsetlocale(LC_CTYPE, TEXT("chs"));
  _tprintf_s(TEXT("监听"));
  _tprintf_s(TEXT(" [%s]\n"), sDir);
  wprintf_s(L"按 <%c> 退出.\n", 'q');

  while (_getch() != 'q');
  wprintf_s(L"正在退出...\n");
  fsw.Close(1000);

  return 0;
}
