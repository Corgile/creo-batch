#include <filesystem>
#include <fstream>
#include <set>
#include <queue>
#include <sstream>
#include <iostream>
#include <conio.h>
#include <thread>
#include "file-watcher.hpp"

namespace fs = std::filesystem;

void ProcessCurrentDirectory(const fs::path &current_dir);

fs::path gblib_dir;
FileSystemWatcher *fsw;

void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam) {
  SetConsoleOutputCP(65001);
  fs::path modified_item(_filename);
  if (modified_item.filename().string().find(".swp") != std::string::npos) return;
  if (modified_item.filename() == "GBLib.ctg.1") return;

  static FileSystemWatcher::ACTION pre_act = FileSystemWatcher::ACTION_ERRSTOP;
  switch (action) {
    case FileSystemWatcher::ACTION_ADDED:
      std::printf("\n\033[32m\t++ 添加 [%s]\033[0m", modified_item.string().c_str());
      break;
    case FileSystemWatcher::ACTION_REMOVED:
      std::printf("\n\033[31m\tX 删除 [%s]\033[0m", modified_item.string().c_str());
      break;
    case FileSystemWatcher::ACTION_RENAMED_OLD:
      std::printf("\n\033[33m\t<> 将 [%s] 重命名为 ", modified_item.string().c_str());
      break;
    case FileSystemWatcher::ACTION_RENAMED_NEW:
      if (pre_act == FileSystemWatcher::ACTION_RENAMED_OLD) {
        std::printf("[%s]\033[0m", modified_item.string().c_str());
      }
      break;
    case FileSystemWatcher::ACTION_MODIFIED:
      std::printf(",\033[35m *导致 [%s] 被修改\033[0m,", modified_item.string().c_str());
      if (fs::is_directory(gblib_dir / modified_item)) {
        std::printf("\033[36m ->更新 [%s] 里的.mnu文件\033[0m",
                    modified_item.string().c_str());
        fsw->Close();
        ProcessCurrentDirectory(gblib_dir / modified_item);
        fsw->Run();
      }
      break;
    case FileSystemWatcher::ACTION_ERRSTOP:
    default:
      break;
  }
  pre_act = action;
  system("%PRO_LIBRARY_DIR%\\pro_build_library_ctg.exe > nul");
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
  for (const auto &entry: fs::directory_iterator(current_dir)) {
    if (entry.is_regular_file()) {
      if (entry.path().filename().string().find(".mnu") != std::string::npos) {
        fs::remove(entry);
      }
    }
  }
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

  if (!proLibraryDir) {
    std::cout << "环境变量 PRO_LIBRARY_DIR 未设置" << std::endl;
    if (argc == 1) {
      std::cout << ", 程序并且缺少必要路径参数, 正在退出程序.." << std::endl;
      exit(EXIT_FAILURE);
    } else {
      gblib_dir = fs::path(argv[1]);
      if (!fs::exists(gblib_dir)) {
        std::cout << "路径 [" << gblib_dir.string() << "] 不存在, 正在退出程序.." << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  } else {
    gblib_dir = fs::path(proLibraryDir);
  }

  std::cout << "- 正在生成 .mnu，请稍候" << std::endl;
  TraverseDirectoryNR(gblib_dir);
  system("%PRO_LIBRARY_DIR%\\pro_build_library_ctg.exe > nul");
  std::cout << "- 创建 .mnu 已完成" << std::endl;

  auto sDir = gblib_dir.string();
  DWORD dwNotifyFilter =
      FileSystemWatcher::FILTER_FILE_NAME
      | FileSystemWatcher::FILTER_DIR_NAME
      | FileSystemWatcher::FILTER_LAST_ACCESS_NAME;


  fsw = new FileSystemWatcher(sDir.c_str(), true, dwNotifyFilter, &MyCallback, nullptr);
  if (!fsw->Run()) return -1;

  std::cout << "- 监听 [" << sDir << "] 的变化, 按 q 退出.\n";

  while (_getch() != 'q');
  std::cout << "正在退出...\n";
  fsw->Close(1000);


  return 0;
}
