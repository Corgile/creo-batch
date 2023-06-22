#include "file-watcher.hpp"
#include <cassert>
#include <iostream>
#include <thread>

FileSystemWatcher::FileSystemWatcher(LPCTSTR dir, bool bWatchSubtree, DWORD dwNotifyFilter, LPDEALFUNCTION callback,
                                     LPVOID lParam) {
  assert(FILTER_FILE_NAME == FILE_NOTIFY_CHANGE_FILE_NAME);
  assert(FILTER_DIR_NAME == FILE_NOTIFY_CHANGE_DIR_NAME);
  assert(FILTER_ATTR_NAME == FILE_NOTIFY_CHANGE_ATTRIBUTES);
  assert(FILTER_SIZE_NAME == FILE_NOTIFY_CHANGE_SIZE);
  assert(FILTER_LAST_WRITE_NAME == FILE_NOTIFY_CHANGE_LAST_WRITE);
  assert(FILTER_LAST_ACCESS_NAME == FILE_NOTIFY_CHANGE_LAST_ACCESS);
  assert(FILTER_CREATION_NAME == FILE_NOTIFY_CHANGE_CREATION);
  assert(FILTER_SECURITY_NAME == FILE_NOTIFY_CHANGE_SECURITY);

  assert(ACTION_ADDED == FILE_ACTION_ADDED);
  assert(ACTION_REMOVED == FILE_ACTION_REMOVED);
  assert(ACTION_MODIFIED == FILE_ACTION_MODIFIED);
  assert(ACTION_RENAMED_OLD == FILE_ACTION_RENAMED_OLD_NAME);
  assert(ACTION_RENAMED_NEW == FILE_ACTION_RENAMED_NEW_NAME);
  m_bWatchSubtree = bWatchSubtree;
  m_dwNotifyFilter = dwNotifyFilter;
  m_DealFun = callback;
  m_DealFunParam = lParam;
  m_bRequestStop = false;
  m_dir = dir;
  m_hThread = nullptr;
  m_hDir = INVALID_HANDLE_VALUE;
}

FileSystemWatcher::~FileSystemWatcher() {
  this->suspend();
}

bool FileSystemWatcher::start() {
  this->stop();
  //m_dir目录不能以'\'结尾，否则监测不到dir目录被删除，不以\结尾，可以检测到（仅限于空目录时）
  m_hDir = CreateFile(
      m_dir,
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS,
      nullptr
  );
  if (INVALID_HANDLE_VALUE == m_hDir) exit(EXIT_FAILURE);
  DWORD ThreadId;
  m_hThread = CreateThread(nullptr, 0, Routine, this, 0, &ThreadId);
  if (nullptr == m_hThread) {
    CloseHandle(m_hDir);
    m_hDir = INVALID_HANDLE_VALUE;
  }
  return nullptr != m_hThread;
}

void FileSystemWatcher::stop(DWORD dwMilliseconds) {
  if (nullptr != m_hThread) {
    m_bRequestStop = true;
    if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread, dwMilliseconds)) {
      // FIXME: 不推荐的方法
      TerminateThread(m_hThread, 0);
    }
    CloseHandle(m_hThread);
    m_hThread = nullptr;
  }
  if (INVALID_HANDLE_VALUE != m_hDir) {
    CloseHandle(m_hDir);
    m_hDir = INVALID_HANDLE_VALUE;
  }
}

DWORD WINAPI FileSystemWatcher::Routine(LPVOID lParam) {
  FileSystemWatcher &obj = *(FileSystemWatcher *) lParam;

  BYTE buf[2 * (sizeof(FILE_NOTIFY_INFORMATION) + 2 * MAX_PATH) + 2];
  auto pNotify = (FILE_NOTIFY_INFORMATION *) buf;
  DWORD BytesReturned;
  while (!obj.m_bRequestStop) {
    if (ReadDirectoryChangesW(obj.m_hDir,
                              pNotify,
                              sizeof(buf) - 2,
                              obj.m_bWatchSubtree,
                              obj.m_dwNotifyFilter,
                              &BytesReturned,
                              nullptr,
                              nullptr)) // 无限等待，应当使用异步方式
    {
      int i = 0;
      for (FILE_NOTIFY_INFORMATION *p = pNotify; p && i < 4;) {
        WCHAR c = p->FileName[p->FileNameLength >> 1];
        p->FileName[p->FileNameLength >> 1] = L'\0';
        obj.m_DealFun((ACTION) p->Action, p->FileName, obj.m_DealFunParam);
        p->FileName[p->FileNameLength >> 1] = c;
        if (p->NextEntryOffset) {
          p = (PFILE_NOTIFY_INFORMATION) ((BYTE *) p + p->NextEntryOffset);
        } else {
          p = nullptr;
        }
        i++;
      }
    } else {
      obj.m_DealFun((ACTION) ACTION_ERRSTOP, nullptr, obj.m_DealFunParam);
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  return 0;
}

void FileSystemWatcher::suspend() {
  DWORD suspendCount = SuspendThread(m_hThread);
  if (suspendCount == (DWORD) -1) {
    std::cout << "file-watcher thread failed to suspend." << std::endl;
    exit(EXIT_FAILURE);
  }
}

