/*
This file is part of Universal Modding Engine.


Universal Modding Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Universal Modding Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Universal Modding Engine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "uMod_Logging.h"

#include <windows.h>
#include <cstdio>
#include <wx/filename.h>

namespace {
FILE *g_LogFile = NULL;
CRITICAL_SECTION g_LogLock;
bool g_LogInit = false;
}

wxString GetLogPath(void)
{
  wchar_t module_path[MAX_PATH];
  DWORD len = GetModuleFileNameW(NULL, module_path, MAX_PATH);
  if (len == 0) return L"uMod_Reforged_debug.log";
  wxString path(module_path);
  wxString dir = path.BeforeLast('\\');
  if (dir.empty()) dir = ".";
  wxFileName log_path(dir, "uMod_Reforged_debug.log");
  return log_path.GetFullPath();
}

void InitLogger(void)
{
  if (g_LogInit) return;
  InitializeCriticalSection(&g_LogLock);
  g_LogInit = true;

  wxString path = GetLogPath();
  if (_wfopen_s(&g_LogFile, path.wc_str(), L"wt") != 0) g_LogFile = NULL;
  if (g_LogFile != NULL)
  {
    setvbuf(g_LogFile, NULL, _IONBF, 0);
    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(g_LogFile, L"[Init %04d-%02d-%02d %02d:%02d:%02d.%03d] Logging started\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    fwprintf(g_LogFile, L"[Init] Log path: %ls\n", path.wc_str());
  }
}

void ShutdownLogger(void)
{
  if (!g_LogInit) return;
  EnterCriticalSection(&g_LogLock);
  if (g_LogFile != NULL)
  {
    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(g_LogFile, L"[Shutdown %04d-%02d-%02d %02d:%02d:%02d.%03d] Logging stopped\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    fclose(g_LogFile);
    g_LogFile = NULL;
  }
  LeaveCriticalSection(&g_LogLock);
  DeleteCriticalSection(&g_LogLock);
  g_LogInit = false;
}

void LogMessage(const wxString &message)
{
  if (!g_LogInit || g_LogFile == NULL) return;
  EnterCriticalSection(&g_LogLock);
  SYSTEMTIME st;
  GetLocalTime(&st);
  fwprintf(g_LogFile, L"[%04d-%02d-%02d %02d:%02d:%02d.%03d][T%lu] %ls\n",
           st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
           GetCurrentThreadId(), message.wc_str());
  LeaveCriticalSection(&g_LogLock);
}
