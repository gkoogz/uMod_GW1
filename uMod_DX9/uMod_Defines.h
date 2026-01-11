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



#ifndef uMod_DEFINES_H_
#define uMod_DEFINES_H_

#include <cstdarg>
#include <cstring>



#ifdef LOG_MESSAGE
extern FILE *gl_File;

inline void uMod_WriteLog(const char *format, ...)
{
  if (gl_File == NULL) return;
  SYSTEMTIME st;
  GetLocalTime(&st);
  fprintf(gl_File, "[%04d-%02d-%02d %02d:%02d:%02d.%03d][T%lu] ",
          st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
          GetCurrentThreadId());
  va_list args;
  va_start(args, format);
  vfprintf(gl_File, format, args);
  va_end(args);
  fflush(gl_File);
}

inline void uMod_GetLogPath(char *path, size_t size)
{
  char module_path[MAX_PATH];
  DWORD len = GetModuleFileNameA(NULL, module_path, MAX_PATH);
  if (len == 0 || len >= MAX_PATH)
  {
    strncpy_s(path, size, "uMod_Reforged_DI_debug.log", _TRUNCATE);
    return;
  }
  const char *last_slash = strrchr(module_path, '\\');
  if (last_slash == NULL) last_slash = strrchr(module_path, '/');
  size_t dir_len = last_slash ? static_cast<size_t>(last_slash - module_path + 1) : 0u;
  if (dir_len >= size) dir_len = size - 1;
  strncpy_s(path, size, module_path, dir_len);
  strncat_s(path, size, "uMod_Reforged_DI_debug.log", _TRUNCATE);
}

#define Message(...) uMod_WriteLog(__VA_ARGS__)
#define OpenMessage(...) \
  { \
    char log_path[MAX_PATH]; \
    uMod_GetLogPath(log_path, MAX_PATH); \
    if (fopen_s(&gl_File, log_path, "wt")) gl_File = NULL; \
    else uMod_WriteLog("DI log start (path=%s)\n", log_path); \
  }
#define CloseMessage(...) {if (gl_File!=NULL) fclose(gl_File);}


#else
#define OpenMessage(...)
#define Message(...)
#define CloseMessage(...)
#endif


#ifdef __CDT_PARSER__
typedef unsigned long DWORD64;
typedef unsigned long DWORD32;

#define STDMETHOD(method)     virtual HRESULT method
#define STDMETHOD_(ret, method)  virtual ret method
#define sprintf_s(...)
#define fprintf(...)
#define fclose(...)
#define fseek(...)
#define ftell(...) 0
#define fflush(...)
typedef LONG HRESULT;

#define UNREFERENCED_PARAMETER(...)
#endif


#endif /* uMod_DEFINES_H_ */
