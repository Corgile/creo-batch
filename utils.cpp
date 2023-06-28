//
// Created by xhl on 2023/6/28.
//
#include "utils.hpp"

std::wstring utils::TrimNumSuffix(std::wstring str) {
  std::size_t dotPosition = str.find_last_of('.');
  if (dotPosition == std::wstring::npos) { return str; }
  std::wstring suffix = str.substr(dotPosition + 1);// 检查后缀是否是数字(如果是)
  if (suffix.find_first_not_of(L"0123456789") == std::wstring::npos) {
    str = str.substr(0, dotPosition);
  }
  return str;
}

bool utils::StartsWith(const std::wstring &str, const std::wstring &prefix) {
  if (prefix.length() > str.length())
    return false;
  return std::equal(prefix.begin(), prefix.end(), str.begin());
}

bool utils::EndsWith(const std::wstring &str, const std::wstring &suffix) {
  if (suffix.length() > str.length())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

std::string utils::ToGBK(const std::wstring &utf16Text) {
  int requiredSize = WideCharToMultiByte(0, 0, utf16Text.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (requiredSize <= 0) {
    // Failed to calculate required buffer size
    return {};
  }
  std::string gbkText(requiredSize, '\0');
  if (WideCharToMultiByte(0, 0, utf16Text.c_str(), -1, &gbkText[0], requiredSize, nullptr, nullptr) != 0) {
    gbkText.resize(requiredSize - 1);// 去除结尾的 null 字符
    return gbkText;
  }
  // Failed to convert UTF-16 to GBK
  return {};
}
