// Start a process relative to the .exe's location.

// gcc -m32 -Wall -O3 -mwindows CreateProcess.c -o a.exe -s
// cl /Ox /GS /guard:cf CreateProcess.c /link /out:a.exe user32.lib

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define APPEND_EXE "bin\\win32\\program.exe"
#define APPEND_SIZE 32

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  LPCTSTR title = "Error";
  UINT type = MB_ICONERROR | MB_OK;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  char moduleName[MAX_PATH];
  char moduleRootDrive[_MAX_DRIVE];
  char moduleRootDir[_MAX_DIR];
  char command[_MAX_DRIVE + _MAX_DIR + APPEND_SIZE];
  char error[64];
  int rv = 0;

  if (!GetModuleFileName(hInstance, moduleName, MAX_PATH)) {
    MessageBox(0, "Failed calling GetModuleFileName()", title, type);
    return 1;
  }

  if (_splitpath_s(moduleName, moduleRootDrive, _MAX_DRIVE, moduleRootDir, _MAX_DIR, NULL, 0, NULL, 0) != 0) {
    MessageBox(0, "Failed calling _splitpath_s()", title, type);
    return 1;
  }

  snprintf(command, _MAX_DRIVE + _MAX_DIR + APPEND_SIZE - 1, "%s%s" APPEND_EXE, moduleRootDrive, moduleRootDir);

  SecureZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  SecureZeroMemory(&pi, sizeof(pi));

  if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    snprintf(error, 63, "CreateProcess() failed (%ld).", (long)GetLastError());
    MessageBox(0, error, title, type);
    return 1;
  }

  rv = WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  if (rv == WAIT_ABANDONED) {
    MessageBox(0, "Process abandoned.", title, type);
    rv = 1;
  } else if (rv == WAIT_TIMEOUT) {
    MessageBox(0, "Process time-out error.", title, type);
    rv = 1;
  } else if (rv == WAIT_FAILED) {
    snprintf(error, 63, "Process failed (%ld).", (long)GetLastError());
    MessageBox(0, error, title, type);
    rv = 1;
  }

  return rv;
}

