//
// Created by corgi on 2023年6月13日.
//
#include <iostream>
#include <Windows.h>

void MonitorDirectoryChanges(const std::wstring &directoryPath) {
  HANDLE hDirectory = CreateFileW(
     directoryPath.c_str(),
     FILE_LIST_DIRECTORY,
     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
     NULL,
     OPEN_EXISTING,
     FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
     NULL
  );

  if (hDirectory == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to open directory: " << GetLastError() << std::endl;
    return;
  }

  constexpr DWORD bufferSize = 4096;
  BYTE buffer[bufferSize];
  DWORD bytesReturned;

  OVERLAPPED overlapped;
  ZeroMemory(&overlapped, sizeof(overlapped));
  overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  while (true) {
    if (!ReadDirectoryChangesW(
       hDirectory,
       buffer,
       bufferSize,
       TRUE,
       FILE_NOTIFY_CHANGE_FILE_NAME
       | FILE_NOTIFY_CHANGE_DIR_NAME
       | FILE_NOTIFY_CHANGE_LAST_WRITE,
       &bytesReturned,
       &overlapped,
       nullptr
    )) {
      std::cerr << "Failed to read directory changes: " << GetLastError() << std::endl;
      break;
    }

    DWORD waitResult = WaitForSingleObject(overlapped.hEvent, INFINITE);
    if (waitResult == WAIT_OBJECT_0) {
      DWORD numberOfBytesTransferred;
      if (GetOverlappedResult(hDirectory, &overlapped, &numberOfBytesTransferred, FALSE)) {
        auto fileInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer);
        while (true) {
          std::wstring filename(fileInfo->FileName, fileInfo->FileNameLength / sizeof(WCHAR));
          //std::wcout << "Detected change: " << filename << std::endl;

          fileInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(
             reinterpret_cast<BYTE *>(fileInfo) + fileInfo->NextEntryOffset
          );

          unsigned long pre_act = 0;
          switch (fileInfo->Action) {
            case FILE_ACTION_ADDED:
              wprintf_s(L"\033[32m Add:\t%s\033[0m\n", fileInfo->FileName);
              // std::wcout << "\033[32m添加:\t" << filename << "\033[0m\n";
              break;
            case FILE_ACTION_REMOVED:
              wprintf_s(L"\033[31m删除: \t%s\033[0m\n", fileInfo->FileName);
              // std::wcout << "\033[31m删除:\t" << filename << "\033[0m\n";
              break;
            case FILE_ACTION_MODIFIED:
              //if (pre_act == FILE_ACTION_MODIFIED) {
              wprintf_s(L"\033[33m修改:\t%s\033[0m\n", fileInfo->FileName);
              // std::wcout << "\033[33m修改:\t" << filename << "\033[0m\n";
              //}
              break;
            case FILE_ACTION_RENAMED_OLD_NAME:
              wprintf_s(L"\033[34m将 [%s] 改名为 ", fileInfo->FileName);
              // std::wcout << "\033[34m将 [" << filename << "] 改名为 ";
              break;
            case FILE_ACTION_RENAMED_NEW_NAME:
              //if (pre_act == FILE_ACTION_RENAMED_OLD_NAME) {
              wprintf_s(L"[%s]\033[0m\n", fileInfo->FileName);
              // std::wcout << "[" << filename << "]\033[0m\n";
              //}
              break;
            default:
              wprintf_s(L"---错误---%s\n", fileInfo->FileName);
              // std::wcout << "---错误---" << filename << "\n";
              break;
          }
          pre_act = fileInfo->Action;
          if (fileInfo->NextEntryOffset == 0) { break; }
        }
      } else {
        std::cerr << "Failed to get overlapped result: " << GetLastError() << std::endl;
        break;
      }
    } else if (waitResult == WAIT_FAILED) {
      std::cerr << "Wait for directory changes failed: " << GetLastError() << std::endl;
      break;
    }

    // Reset the overlapped structure for the next read operation
    ResetEvent(overlapped.hEvent);
  }

  CloseHandle(overlapped.hEvent);
  CloseHandle(hDirectory);
}

int main() {
  std::wstring directoryPath = L"C:\\Users\\gzhu03\\IdeaProjects\\cmd-script\\example"; // 替换为要监视的目录路径
  MonitorDirectoryChanges(directoryPath);

  return 0;
}
