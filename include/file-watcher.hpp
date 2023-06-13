#ifndef FILE_SYSTEM_WATCHER_HPP
#define FILE_SYSTEM_WATCHER_HPP

#if(_WIN32_WINNT < 0x0400)
#define _WIN32_WINNT 0x0400
#endif

#include <windows.h>

/*
* Value	含义
FILE_NOTIFY_CHANGE_FILE_NAME
0x00000001
对受监视的目录或子树中文件名的任意更改都会导致返回一个更改通知等待操作。 更改包括重命名、创建或删除文件。
FILE_NOTIFY_CHANGE_DIR_NAME
0x00000002
监视目录或子树中的任何目录名称更改都会导致更改通知等待操作返回。 更改包括创建或删除目录。
FILE_NOTIFY_CHANGE_ATTRIBUTES
0x00000004
对受监视的目录或子树中属性的任意更改都会导致返回一个更改通知等待操作。
FILE_NOTIFY_CHANGE_SIZE
0x00000008
对受监视的目录或子树中文件大小的任意更改都会导致返回一个更改通知等待操作。 仅当文件写入磁盘时，操作系统才会检测文件大小的更改。 对于使用大量缓存的操作系统，仅当缓存充分刷新时，才会进行检测。
FILE_NOTIFY_CHANGE_LAST_WRITE
0x00000010
对受监视的目录或子树中文件的上次写入时间的任意更改都会导致返回一个更改通知等待操作。 仅当文件写入磁盘时，操作系统才会检测上次写入时间的更改。 对于使用大量缓存的操作系统，仅当缓存充分刷新时，才会进行检测。
FILE_NOTIFY_CHANGE_LAST_ACCESS
0x00000020
对所监视目录或子树中文件的上次访问时间所做的任何更改都会导致更改通知等待操作返回。
FILE_NOTIFY_CHANGE_CREATION
0x00000040
对所监视目录或子树中的文件的创建时间所做的任何更改都会导致更改通知等待操作返回。
FILE_NOTIFY_CHANGE_SECURITY
0x00000100
监视目录或子树中的任何安全描述符更改都会导致更改通知等待操作返回。
*/

class FileSystemWatcher {
public:
  enum Filter {
    FILTER_FILE_NAME = 0x00000001, // add/remove/rename
    FILTER_DIR_NAME = 0x00000002, // add/remove/rename
    FILTER_ATTR_NAME = 0x00000004,
    FILTER_SIZE_NAME = 0x00000008,
    FILTER_LAST_WRITE_NAME = 0x00000010, // timestamp
    FILTER_LAST_ACCESS_NAME = 0x00000020, // timestamp
    FILTER_CREATION_NAME = 0x00000040, // timestamp
    FILTER_SECURITY_NAME = 0x00000100
  };
  enum ACTION {
    ACTION_ERRSTOP = -1,
    ACTION_ADDED = 0x00000001,
    ACTION_REMOVED = 0x00000002,
    ACTION_MODIFIED = 0x00000003,
    ACTION_RENAMED_OLD = 0x00000004,
    ACTION_RENAMED_NEW = 0x00000005
  };

  typedef void(WINAPI *LPDEALFUNCTION)(ACTION act, LPCWSTR filename, LPVOID lParam);

  FileSystemWatcher(LPCTSTR dir, bool bWatchSubtree, DWORD dwNotifyFilter, LPDEALFUNCTION callback, LPVOID lParam);

  ~FileSystemWatcher();

  // LPCTSTR dir: dont end-with "\\"
  bool Run();

  void Close(DWORD dwMilliseconds = INFINITE);

private: // no-impl
  FileSystemWatcher(const FileSystemWatcher &);

  FileSystemWatcher operator=(const FileSystemWatcher);

private:
  HANDLE m_hDir;
  LPCTSTR m_dir;
  DWORD m_dwNotifyFilter;
  bool m_bWatchSubtree;
  HANDLE m_hThread;
  volatile bool m_bRequestStop;
  LPDEALFUNCTION m_DealFun;
  LPVOID m_DealFunParam;

  static DWORD WINAPI Routine(LPVOID lParam);
};

#endif // FILE_SYSTEM_WATCHER_HPP