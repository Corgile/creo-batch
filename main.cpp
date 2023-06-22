#include "utils.hpp"


int main(int argc, char *argv[]) {
  SetConsoleOutputCP(65001);
  const char *proLibraryDir = std::getenv("PRO_LIBRARY_DIR");
  if (!proLibraryDir) {
    std::cout << "环境变量 PRO_LIBRARY_DIR 未设置" << std::endl;
    if (argc == 1) {
      std::cout << ", 程序并且缺少必要路径参数, 正在退出程序.." << std::endl;
      exit(EXIT_FAILURE);
    } else {
      xhl::cwd_prefix = xhl::fs::path(argv[1]);
      if (!xhl::fs::exists(xhl::cwd_prefix)) {
        std::wcout << "路径 [" << xhl::cwd_prefix.wstring() << "] 不存在, 正在退出程序.." << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  } else {
    xhl::cwd_prefix = xhl::fs::path(proLibraryDir);
  }

  std::cout << "- 正在生成 .mnu，请稍候" << std::endl;
  xhl::TraverseDirectoryNR(xhl::cwd_prefix);
  system("%PRO_LIBRARY_DIR%\\pro_build_library_ctg.exe > nul");
  std::cout << "- 创建 .mnu 已完成" << std::endl;
  std::cout << "如需开启自动检测变化功能，按 y/Y. 其他任意键退出程序" << std::endl;

  int keyCode = _getch();
  if (keyCode != 'y' && keyCode != 'Y') {
    SetConsoleOutputCP(936);
    system("pause");
    return 0;
  }

  auto sDir = xhl::cwd_prefix.string();
  DWORD dwNotifyFilter =
     FileSystemWatcher::FILTER_FILE_NAME
     | FileSystemWatcher::FILTER_DIR_NAME
     | FileSystemWatcher::FILTER_LAST_ACCESS_NAME;

  xhl::watcher = new FileSystemWatcher(sDir.c_str(), true, dwNotifyFilter, &xhl::MyCallback, nullptr);
  if (!xhl::watcher->start()) return -1;
  std::cout << "- 正在检测 [" << sDir << "] 的变化, 按 q 退出.\n";
  while (_getch() != 'q');
  xhl::watcher->stop(1);
  SetConsoleOutputCP(936);

  return 0;
}
