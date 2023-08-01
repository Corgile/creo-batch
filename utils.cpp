//
// Created by xhl on 2023/6/28.
//
#include "utils.hpp"

std::wstring utils::TrimNumSuffix(std::wstring wstr) {
  auto dot_position{wstr.find_last_of('.')};
  if (dot_position == std::wstring::npos) { return wstr; }
  auto suffix{wstr.substr(dot_position + 1)};
  // 检查后缀是否是数字(如果是)
  if (suffix.find_first_not_of(L"0123456789") == std::wstring::npos) {
    wstr = wstr.substr(0, dot_position);
  }
  return wstr;
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
  int required_size{WideCharToMultiByte(0, 0, utf16Text.c_str(), -1, nullptr, 0, nullptr, nullptr)};
  if (required_size <= 0) {
    // Failed to calculate required buffer size
    return {};
  }
  std::string gbkText(required_size, '\0');
  if (WideCharToMultiByte(0, 0, utf16Text.c_str(), -1, &gbkText[0], required_size, nullptr, nullptr) not_eq 0) {
    gbkText.resize(required_size - 1);// 去除结尾的 null 字符
    return gbkText;
  }
  // Failed to convert UTF-16 to GBK
  return {};
}
