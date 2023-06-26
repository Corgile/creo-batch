//// Created by corgi on 2023年6月23日.

#ifndef CMD_SCRIPT_UTILS_HPP
#define CMD_SCRIPT_UTILS_HPP
#ifdef AUTO_DETECT
#include "file-watcher.hpp"
#include <conio.h>
#else

#include <windows.h>
#include <stringapiset.h>

#endif

#include <filesystem>
#include <fstream>
#include <set>
#include <queue>
#include <sstream>
#include <iostream>

#include <optional>


namespace xhl {
    namespace fs = std::filesystem;
    fs::path gb_lib_home;
#ifdef AUTO_DETECT
    FileSystemWatcher *watcher;
#endif

    void TraverseDirectoryNR(const fs::path &directory);

    void GenerateMenuFileInCurrentDir(const fs::path &current_dir);

    bool StartsWith(const std::wstring &str, const std::wstring &prefix);

    bool EndsWith(const std::wstring &str, const std::wstring &suffix);

    bool IsAllowedFile(const std::wstring &file_name, bool hasAsm);

    std::string ToGBK(std::wstring &utf16Text);

    std::wstring TrimNumSuffix(std::wstring str);

    class dir_item;

    class ls_result_holder;

#ifdef AUTO_DETECT
    void WINAPI MyCallback(FileSystemWatcher::ACTION action, LPCWSTR _filename, LPVOID lParam);
#endif

}

#endif//CMD_SCRIPT_UTILS_HPP