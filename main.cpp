#include "utils.hpp"
#include "dir_item.hpp"
#include <iostream>

void TraverseDirectoryNR(const utils::fs::path &directory) {
  std::queue<utils::fs::path> queue;
  queue.push(directory);
  while (!queue.empty()) {
    utils::fs::path current{queue.front()};
    queue.pop();
    if (utils::fs::is_regular_file(current)) continue;

    dir_item folder(current);
    for (const auto &entry: utils::fs::directory_iterator(current)) {
      auto wstr_filename{entry.path().filename().wstring()};
      if (utils::EndsWith(wstr_filename, L".mnu")) {
        utils::fs::remove(entry);
        continue;
      }
      auto should_rename{entry.is_directory() && not utils::StartsWith(wstr_filename, L"A-")};
      auto renamed_path{entry.path().parent_path() / (L"A-" + wstr_filename)};
      if (should_rename) {
        utils::fs::rename(entry, renamed_path);
      }
      queue.emplace(should_rename ? renamed_path : entry);
      folder.add_child(should_rename ? renamed_path : entry);
    }
    folder.dump_to_file();
  }
}

int main(int argc, char *argv[]) {
  SetConsoleOutputCP(65001);
  utils::fs::path gb_lib_home;
  const char *proLibraryDir = std::getenv("PRO_LIBRARY_DIR");
  if (!proLibraryDir) {
    std::cout << "环境变量 PRO_LIBRARY_DIR 未设置" << std::endl;
    if (argc == 1) {
      std::cout << ", 程序并且缺少必要路径参数, 正在退出程序.." << std::endl;
      exit(EXIT_FAILURE);
    } else {
      gb_lib_home = utils::fs::path(argv[1]);
      if (!utils::fs::exists(gb_lib_home)) {
        std::wcout << "路径 [" << gb_lib_home.wstring() << "] 不存在, 正在退出程序.." << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  } else {
    gb_lib_home = utils::fs::path(proLibraryDir);
  }

  std::cout << "- 正在生成 .mnu 文件" << std::endl;
  TraverseDirectoryNR(gb_lib_home);
  std::cout << "- OK\n- 运行 pro_build_library_ctg.exe" << std::endl;
  system("%PRO_LIBRARY_DIR%\\pro_build_library_ctg.exe > nul");
  std::cout << "- 完成" << std::endl;
  SetConsoleOutputCP(936);

  return 0;
}
