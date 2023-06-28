//// Created by corgi on 2023年6月23日.

#ifndef CMD_SCRIPT_UTILS_HPP
#define CMD_SCRIPT_UTILS_HPP

#include <stringapiset.h>
#include <filesystem>
#include <fstream>
#include <set>
#include <queue>


namespace utils {
  namespace fs = std::filesystem;

  std::wstring TrimNumSuffix(std::wstring str);

  bool StartsWith(const std::wstring &str, const std::wstring &prefix);

  bool EndsWith(const std::wstring &str, const std::wstring &suffix);

  std::string ToGBK(const std::wstring &utf16Text);

}

#endif//CMD_SCRIPT_UTILS_HPP