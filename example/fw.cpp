//
// Created by gzhu03 on 12/6/2023.
//
#include <fstream>

int main() {
  std::ofstream mnuStream("./file.txt", std::ios::out | std::ios::app);
  mnuStream << "hello" << "\n#\n#\n";
  mnuStream << "hello" << "\n#\n#\n";
  mnuStream << "hello" << "\n#\n#\n";
  mnuStream.close();
  return 0;
}