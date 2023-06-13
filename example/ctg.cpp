//
// Created by gzhu03 on 13/6/2023.
//
#include <windows.h>
#include <thread>

int main() {
  std::thread worker([]() {
    system("%PRO_LIBRARY_DIR%\\pro_build_library_ctg.exe > nul");
  });
  worker.join();
  return 0;
}