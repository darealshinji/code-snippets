/**
 * Wrapper to launch GrapplingHook.exe:
 * sets CD key in preferences to a default value
 * and adds OpenAL dlls to PATH
 */

// On Linux you simply use a shell script like this:
//
// #!/bin/sh
// email="free@grapplinghook.de"
// cdkey="5Q2CW-PLX3E-2T2R1-61SPH-NZ13P"
// tuser="fsGFBTEI7XIkjzkh/cpZug\\=\\="
// cfg="$HOME/.GrapplingHook/properties.cfg"
// mkdir -p "$HOME/.GrapplingHook"
// touch "$cfg"
// sed -i "/^EMAIL=.*/d; /^CDKEY=.*/d; /^TUSER=.*/d" "$cfg"
// printf "\nEMAIL=$email\nCDKEY=$cdkey\nTUSER=$tuser\n" >> "$cfg"
// cd "${0%/*}"
// java -Xms128m -Xmx256m -Djava.library.path=lib -Dfile,encoding=UTF-8 -cp lib/bin.jar:lib/lwjgl.jar:lib/lwjgl_util.jar:lib/jogg-0.0.7.jar:lib/jorbis-0.0.15.jar ghook/GHookGame

/**
 * Compile with GCC/MinGW:
 *   windres ghookLaunch.res ghookLaunch_res.o
 *   gcc -m32 -static -Wall -O3 -mwindows ghookLaunch.c ghookLaunch_res.o -o launch.exe -luser32 -lshell32 -lshlwapi -s
 *
 * Compile with MSVC:
 *   cl /Ox /GS /guard:cf ghookLaunch.c /link /out:launch.exe ghookLaunch.res user32.lib shell32.lib shlwapi.lib
 */

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>

/* official freeware key:
 * https://www.speedrungames.com/?p=725
 * https://grapplinghook.de/download_form.php
 */
#define EMAIL "free@grapplinghook.de"
#define CDKEY "5Q2CW-PLX3E-2T2R1-61SPH-NZ13P"
#define TUSER "fsGFBTEI7XIkjzkh/cpZug\\=\\="

#define BUFSIZE 32767
#define CUSTOM_MAX_PATH 4096
#define CUSTOM_MAX_DRIVE 16

#define die(MSG) \
  snprintf(error, 63, MSG " failed: error code %ld", (long)GetLastError()); \
  MessageBox(0, error, title, type); \
  exit(1);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
  LPCTSTR title = "Error";
  UINT type = MB_ICONERROR | MB_OK;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  DWORD dwRet;
  HANDLE hFile;

  char error[64];
  char moduleName[CUSTOM_MAX_PATH];
  char moduleRootDrive[CUSTOM_MAX_DRIVE];
  char moduleRootDir[CUSTOM_MAX_PATH];
  char szPath[CUSTOM_MAX_PATH];
  char optlib[CUSTOM_MAX_PATH];
  char cfg[CUSTOM_MAX_PATH];
  char oldEnv[BUFSIZE];
  char newEnv[BUFSIZE];

  char configBuf[BUFSIZE];
  char keyBuf[] = "EMAIL=" EMAIL "\n"
                  "CDKEY=" CDKEY "\n"
                  "TUSER=" TUSER "\n";



  /* move to .exe directory */

  if (!GetModuleFileName(hInstance, moduleName, CUSTOM_MAX_PATH))
  {
    die("GetModuleFileName()");
  }

  if (0 != _splitpath_s(moduleName,
                        moduleRootDrive,
                        CUSTOM_MAX_DRIVE,
                        moduleRootDir,
                        CUSTOM_MAX_PATH,
                        NULL, 0,
                        NULL, 0))
  {
    die("_splitpath_s()");
  }

  snprintf(szPath, CUSTOM_MAX_PATH - 1, "%s%s", moduleRootDrive, moduleRootDir);

  if (!SetCurrentDirectory(szPath))
  {
    die("SetCurrentDirectory()");
  }

  snprintf(optlib, CUSTOM_MAX_PATH - 1, "%s\\optlib", szPath);



  /* create/update config file */

  SecureZeroMemory(&szPath, sizeof(szPath));

  if (0 == SHGetFolderPath(NULL,
                           CSIDL_APPDATA,
                           NULL,
                           0,
                           szPath))
  {
    FILE *fp = NULL;
    char line[100];

    snprintf(szPath, CUSTOM_MAX_PATH - 1, "%s\\.GrapplingHook", szPath);
    snprintf(cfg, CUSTOM_MAX_PATH - 1, "%s\\properties.cfg", szPath);

    CreateDirectory(szPath, NULL);

    if (PathFileExists(cfg) && fopen_s(&fp, cfg, "r") == 0)
    {
      while (!feof(fp))
      {
        if (fgets(line, 100, fp) &&
            line[0] != '\n' &&
            strncmp(line, "EMAIL=", 6) != 0 &&
            strncmp(line, "CDKEY=", 6) != 0 &&
            strncmp(line, "TUSER=", 6) != 0)
        {
          strncat_s(configBuf, BUFSIZE, line, _TRUNCATE);

          if (configBuf[strlen(configBuf) - 1] != '\n')
          {
            strncat_s(configBuf, BUFSIZE, "\n", 1);
          }
        }
      }
      fclose(fp);
    }

    strncat_s(configBuf, BUFSIZE, keyBuf, _TRUNCATE);

    hFile = CreateFile(cfg,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
      WriteFile(hFile, &configBuf, strlen(configBuf), NULL, NULL);
      CloseHandle(hFile);
    }
  }



  /* enable sound by appending "optlib" to PATH */

  dwRet = GetEnvironmentVariable("PATH", oldEnv, BUFSIZE);

  if (dwRet > 0 && (dwRet + strlen(optlib) + 1) < BUFSIZE)
  {
    snprintf(newEnv, BUFSIZE - 1, "%s;%s", oldEnv, optlib);
    SetEnvironmentVariable("PATH", newEnv);
  }



  /* start the game */

  SecureZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  SecureZeroMemory(&pi, sizeof(pi));

  if (!CreateProcess("GrapplingHook.exe",
                     "GrapplingHook.exe",
                     NULL,
                     NULL,
                     FALSE,
                     0,
                     NULL,
                     NULL,
                     &si,
                     &pi))
  {
    die("CreateProcess()");
  }

  dwRet = WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  switch (dwRet)
  {
    case WAIT_ABANDONED:
      MessageBox(0, "Process abandoned.", title, type);
      dwRet = 1;
      break;
    case WAIT_TIMEOUT:
      MessageBox(0, "Process time-out error.", title, type);
      dwRet = 1;
      break;
    case WAIT_FAILED:
      die("Process: WaitForSingleObject()");
      break;
  }

  return dwRet;
}

