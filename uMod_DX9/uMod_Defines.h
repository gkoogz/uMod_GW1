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



#ifdef LOG_MESSAGE
extern FILE *gl_File;
extern HINSTANCE gl_hThisInstance;

static inline FILE *uMod_OpenLogFile()
{
  wchar_t module_path[MAX_PATH];
  DWORD len = GetModuleFileNameW(gl_hThisInstance, module_path, MAX_PATH);
  if (len == 0 || len >= MAX_PATH) return NULL;

  DWORD pos = len;
  while (pos > 0)
  {
    if (module_path[pos - 1] == L'\\' || module_path[pos - 1] == L'/') break;
    pos--;
  }
  module_path[pos] = 0;
  wcscat_s(module_path, L"uMod_log.txt");

  FILE *file = NULL;
  if (_wfopen_s(&file, module_path, L"wt") != 0) return NULL;
  fprintf(file, "DI 40: 0000000\n");
  return file;
}

#define Message(...) {if (gl_File!=NULL) {fprintf( gl_File, __VA_ARGS__); fflush(gl_File);}}
#define OpenMessage(...) {gl_File = uMod_OpenLogFile();}
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
