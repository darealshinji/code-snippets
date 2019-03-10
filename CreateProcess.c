// Start a process relative to the .exe's location.

// i686-w64-mingw32-gcc -Wall -O3 -mwindows -municode CreateProcess.c -s -static


// relative path to the main binary to start
#define APPEND_EXE "prog.exe"

// if defined, the program will check if it's running on
// a 64 bit system and then start the 64 bit version
#define APPEND_EXE64 "prog64.exe"

/* Compile test programs from this:
#include <windows.h>
int main(int argc, char *argv[]) {
  MessageBox(0, GetCommandLine(), "Test", MB_OK);
  return 0;
}
*/


#ifndef UNICODE
# define UNICODE
#endif
#ifndef _UNICODE
# define _UNICODE
#endif
#ifndef APPEND_EXE
# error  APPEND_EXE not defined
#endif
#define BUFSIZE 4096
#define PATH_MAX 32767

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


static void show_error_message(int errCode, const wchar_t *str, const wchar_t *title)
{
  int rv;
  wchar_t buf[BUFSIZE];
  wchar_t err[BUFSIZE];

  rv = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      errCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      buf,
                      BUFSIZE,
                      NULL);

  if (rv == 0) {
    _snwprintf_s(err, BUFSIZE, _TRUNCATE, L"%s failed with error code %ld", str, errCode);
  } else {
    _snwprintf_s(err, BUFSIZE, _TRUNCATE, L"%s failed:\n%s", str, buf);
  }

  MessageBoxW(0, err, title, MB_ICONERROR|MB_OK);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
  const wchar_t *title = L"Error: " APPEND_EXE;
  wchar_t modulePath[PATH_MAX];
  wchar_t command[PATH_MAX];

  // get exe path
  if (!GetModuleFileNameW(NULL, modulePath, PATH_MAX)) {
    MessageBoxW(0, L"GetModuleFileName() failed", title, MB_ICONERROR|MB_OK);
    return 1;
  }

  // get dirname
  wchar_t *p = wcsrchr(modulePath, L'\\');
  if (!p) {
    MessageBoxW(0, L"Cannot get executable path!", title, MB_ICONERROR|MB_OK);
    return 1;
  }
  *p = 0;

#ifdef APPEND_EXE64
  SYSTEM_INFO sysi;
  SecureZeroMemory(&sysi, sizeof(sysi));
  GetNativeSystemInfo(&sysi);

  // check if we're on a 64 bit OS
  if (sysi.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
    WIN32_FIND_DATA FindFileData;
    HANDLE handle;

    // check if the 64 bit exe exists
    _snwprintf_s(command, PATH_MAX, _TRUNCATE, L"%s\\" APPEND_EXE64, modulePath);
    handle = FindFirstFileW(command, &FindFileData);

    if (handle == INVALID_HANDLE_VALUE) {
      // not found, use the 32 bit exe
      _snwprintf_s(command, PATH_MAX, _TRUNCATE, L"%s\\" APPEND_EXE, modulePath);
    } else {
      title = L"Error: " APPEND_EXE64;
      FindClose(handle);
    }
  }
#else
  _snwprintf_s(command, PATH_MAX, _TRUNCATE, L"%s\\" APPEND_EXE, modulePath);
#endif

  // append arguments
  _snwprintf_s(command, PATH_MAX, _TRUNCATE, L"%s %s", command, pCmdLine);

  // create process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  SecureZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  SecureZeroMemory(&pi, sizeof(pi));

  if (CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) == FALSE) {
    int errCode = GetLastError();
    show_error_message(errCode, L"CreateProcess()", title);
    return 1;
  }

  int ret = WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  switch(ret) {
    case WAIT_ABANDONED:
      MessageBoxW(0, L"Process abandoned.", title, MB_ICONERROR|MB_OK);
      return 1;
    case WAIT_TIMEOUT:
      MessageBoxW(0, L"Process time-out error.", title, MB_ICONERROR|MB_OK);
      return 1;
    case WAIT_FAILED:
      int errCode = GetLastError();
      show_error_message(errCode, L"Process", title);
      return 1;
  }

  return 0;  // ret == WAIT_OBJECT_0
}

