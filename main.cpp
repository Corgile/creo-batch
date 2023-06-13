#include <conio.h>
#include <filesystem>
#include <fstream>
#include <tchar.h>
#include <set>
#include <queue>
#include <sstream>
#include <iostream>
#include "file-watcher.hpp"

namespace fs = std::filesystem;

void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam) {
  static FileSystemWatcher::ACTION pre_act = FileSystemWatcher::ACTION_ERRSTOP;
  switch (action) {
    case FileSystemWatcher::ACTION_ADDED:
      wprintf_s(L"\t- 添加:\t%s\n", _filename);
      break;
    case FileSystemWatcher::ACTION_REMOVED:
      wprintf_s(L"\t- 删除:\t%s\n", _filename);
      break;
    case FileSystemWatcher::ACTION_MODIFIED:
      if (pre_act == FileSystemWatcher::ACTION_MODIFIED) {
        wprintf_s(L"\t- 修改:\t%s\n", _filename);
      }
      break;
    case FileSystemWatcher::ACTION_RENAMED_OLD:
      wprintf_s(L"\t- 将 [%s] 改名为 ", _filename);
      break;
    case FileSystemWatcher::ACTION_RENAMED_NEW:
      if (pre_act == FileSystemWatcher::ACTION_RENAMED_OLD) {
        wprintf_s(L"[%s]\n", _filename);
      }
      break;
    case FileSystemWatcher::ACTION_ERRSTOP:
    default:
      wprintf_s(L"\t---错误---%s\n", _filename);
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

void ProcessCurrentDirectory(const fs::path &current_dir) {
  // 创建 mnu 文件
  std::ofstream mnu_file(current_dir / (current_dir.filename().wstring() + L".mnu"));
  //  mnu_file.imbue(std::locale("chs"));
  mnu_file << current_dir.filename().string() << "\n#\n#\n";
  // 遍历当前目录下的文件和子目录
  std::set<std::string> items;
  for (const auto &entry: fs::directory_iterator(current_dir)) {
    std::stringstream oss;
    if (entry.is_regular_file()) {
      auto file_name = entry.path().filename().string();
      RemoveVersionNumberSuffix(file_name);
      if (IsAllowedFile(file_name)) {
        oss << file_name << "\nBeiZhu-" << file_name << "\n#\n";
      }
    }
    if (entry.is_directory()) {
      fs::path renamed_path = entry;
      if (!StartsWith(entry.path().filename().string(), "A-")) {
        renamed_path = current_dir / (L"A-" + entry.path().filename().wstring());
        fs::rename(entry.path(), renamed_path);
      }
      auto folder_name = renamed_path.filename().string();
      oss << "/" << folder_name << "\nBeiZhu-" << folder_name.substr(2) << "\n#\n";
    }
    items.emplace(oss.str());
  }
  for (auto &item: items) {
    mnu_file << item;
  }
  items.clear();
}

void TraverseDirectory(const fs::path &directory) {
  if (fs::is_regular_file(directory)) return;
  ProcessCurrentDirectory(directory);
  // 递归遍历子目录
  for (const auto &entry: fs::directory_iterator(directory)) {
    TraverseDirectory(entry.path());
  }
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
//  _tsetlocale(LC_CTYPE, TEXT("chs"));
  SetConsoleOutputCP(65001);
  const char *proLibraryDir = std::getenv("PRO_LIBRARY_DIR");
  fs::path directory;
  if (!proLibraryDir) {
    std::cout << "环境变量 PRO_LIBRARY_DIR 未设置" << std::endl;
    if (argc == 1) {
      std::cout << ", 程序并且缺少必要路径参数, 正在退出程序.." << std::endl;
      exit(EXIT_FAILURE);
    } else {
      directory = fs::path(argv[1]);
      if (!fs::exists(directory)) {
        std::cout << "路径 [" << directory.string() << "] 不存在, 正在退出程序.." << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  } else {
    directory = fs::path(proLibraryDir);
  }

  std::cout << "- 正在生成 .mnu，请稍候" << std::endl;
  TraverseDirectoryNR(directory);
  std::cout << "- 创建 .mnu 已完成" << std::endl;

/*
  LPCTSTR sDir = argv[1];
  DWORD dwNotifyFilter =
      FileSystemWatcher::FILTER_FILE_NAME
      | FileSystemWatcher::FILTER_DIR_NAME
      | FileSystemWatcher::FILTER_LAST_WRITE_NAME;

  FileSystemWatcher fsw;
  bool r = fsw.Run(sDir, true, dwNotifyFilter, &MyCallback, nullptr);
  if (!r) return -1;


  wprintf_s(L"- 监听 [%s] 的变化, ", directory.wstring().c_str());
  wprintf_s(L"按 %c 退出.\n", 'q');

  while (_getch() != 'q');
  wprintf_s(L"正在退出...\n");
  fsw.Close(1000);
  */

  return 0;
}
