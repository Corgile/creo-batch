//
// Created by xhl on 2023/6/28.
//

#ifndef AUTO_CREATE_DIR_ITEM_HPP
#define AUTO_CREATE_DIR_ITEM_HPP

#include "utils.hpp"

class dir_item {
public:
  dir_item(const utils::fs::path &_full_path);

  void dump_to_file();

  void add_child(const utils::fs::path &child);

private:
  utils::fs::path full_path;
  std::wstring short_name;
  std::set<std::wstring> children;
  bool has_asm;
  bool prt_cleared{false};

};


#endif //AUTO_CREATE_DIR_ITEM_HPP
