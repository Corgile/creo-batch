//
// Created by xhl on 2023/6/28.
//

#include "dir_item.hpp"

dir_item::dir_item(const utils::fs::path &_full_path) {
  this->full_path = _full_path;
  this->short_name = _full_path.filename();
  this->has_asm = false;
  this->prt_cleared = false;
}

void dir_item::dump_to_file() {
  if (this->children.empty()) return;
  auto stem{full_path.filename().wstring() + L".mnu"};
  auto mnu_file_name{full_path / utils::fs::path(stem)};
  std::ofstream mnu_file(mnu_file_name, std::ios::out | std::ios::binary);
  std::wstringstream mnu_content;
  // for the title
  mnu_content << this->short_name << L"\n#\n#\n";
  // for the items
  for (const auto &child: children) {
    std::wstring comment{child};
    if (child.find_first_of('/') != std::wstring::npos) {
      comment = child.substr(3);
    }
    mnu_content << child << L"\n"
                << L"备注-" << comment << L"\n"
                << L"#\n";
  }
  mnu_file << utils::ToGBK(mnu_content.str());
  mnu_file.flush();
  mnu_file.close();
}

void dir_item::add_child(const utils::fs::path &child) {
  if (utils::fs::is_directory(child)) {
    this->children.insert(L"/" + child.filename().wstring());
    return;
  }
  auto filename{utils::TrimNumSuffix(child.filename().wstring())};
  bool is_asm = utils::EndsWith(filename, L".asm");
  bool is_prt = utils::EndsWith(filename, L".prt");

  if (is_asm) {
    this->has_asm = true;
    if (prt_cleared) {
      goto emplace;
    }
    for (auto it{children.begin()}; it != children.end();) {
      if (utils::EndsWith(*it, L".prt")) {
        it = children.erase(it);
      } else {
        ++it;
      }
    }
    this->prt_cleared = true;
  }
emplace:
  if (!has_asm && is_prt || has_asm && is_asm) {
    this->children.insert(filename);
  }
}
