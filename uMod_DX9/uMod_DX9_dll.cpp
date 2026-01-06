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

/*


 NEVER USE THIS CODE FOR ILLEGAL PURPOSE


*/


#include "uMod_Main.h"
//#include "detours.h"
//#include "detourxs/detourxs/detourxs.h"

/*
#include "detourxs/detourxs/ADE32.cpp"
#include "detourxs/detourxs/detourxs.cpp"
*/
/*
 * global variable which are not linked external
 */
HINSTANCE             gl_hOriginalDll = NULL;
HINSTANCE             gl_hThisInstance = NULL;
uMod_TextureServer*    gl_TextureServer = NULL;
HANDLE                gl_ServerThread = NULL;

namespace
{
void AppendStartupTrace(const wchar_t *message)
{
  if (gl_hThisInstance == NULL || message == NULL) return;

  wchar_t module_path[MAX_PATH];
  DWORD len = GetModuleFileNameW(gl_hThisInstance, module_path, MAX_PATH);
  if (len == 0 || len >= MAX_PATH) return;

  wchar_t *last_slash = wcsrchr(module_path, L'\\');
  if (last_slash != NULL) *(last_slash + 1) = 0;

  wchar_t log_path[MAX_PATH];
  _snwprintf_s(log_path, _countof(log_path), _TRUNCATE, L"%lsuMod_startup_trace_%lu.txt", module_path, GetCurrentProcessId());

  FILE *file = NULL;
  if (_wfopen_s(&file, log_path, L"a, ccs=UTF-8") == 0 && file != NULL)
  {
    static LONG header_written = 0;
    if (InterlockedCompareExchange(&header_written, 1, 0) == 0)
    {
      wchar_t exe_path[MAX_PATH];
      DWORD exe_len = GetModuleFileNameW(GetModuleHandle(NULL), exe_path, MAX_PATH);
      if (exe_len > 0 && exe_len < MAX_PATH)
      {
        fwprintf(file, L"[%lu] Executable: %ls\n", GetCurrentProcessId(), exe_path);
      }
    }
    fwprintf(file, L"[%lu] %ls\n", GetCurrentProcessId(), message);
    fclose(file);
  }
}
}

typedef IDirect3D9 *(APIENTRY *Direct3DCreate9_type)(UINT);
typedef HRESULT (APIENTRY *Direct3DCreate9Ex_type)(UINT SDKVersion, IDirect3D9Ex **ppD3D);

Direct3DCreate9_type Direct3DCreate9_fn; // we need to store the pointer to the original Direct3DCreate9 function after we have done a detour
Direct3DCreate9Ex_type Direct3DCreate9Ex_fn; // we need to store the pointer to the original Direct3DCreate9 function after we have done a detour



/*
 * global variable which are linked external
 */
unsigned int          gl_ErrorState = 0u;

#ifdef LOG_MESSAGE
FILE*                 gl_File = NULL;
#endif


void Nothing(void) {(void)NULL;}
/*
 * dll entry routine, here we initialize or clean up
 */
BOOL WINAPI DllMain( HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
  UNREFERENCED_PARAMETER(lpReserved);

  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
  {
    InitInstance(hModule);
    break;
  }
  case DLL_PROCESS_DETACH:
  {
    ExitInstance();
    break;
  }
  default:
    break;
  }
    
  return (true);
}


DWORD WINAPI ServerThread( LPVOID lpParam )
{
  UNREFERENCED_PARAMETER(lpParam);
  if (gl_TextureServer!=NULL) gl_TextureServer->MainLoop(); //This is and endless mainloop, it sleep till something is written into the pipe.
  return (0);
}

void InitInstance(HINSTANCE hModule)
{

  DisableThreadLibraryCalls( hModule ); //reduce overhead

  gl_hThisInstance = (HINSTANCE)  hModule;

  AppendStartupTrace(L"InitInstance: begin");
  wchar_t game[MAX_PATH];
  if (!HookThisProgram(game)) return;
  AppendStartupTrace(L"InitInstance: HookThisProgram ok");

  OpenMessage();
  Message("InitInstance: %lu\n", gl_hThisInstance);
  AppendStartupTrace(L"InitInstance: OpenMessage ok");

  gl_TextureServer = new uMod_TextureServer(game); //create the server which listen on the pipe and prepare the update for the texture clients
  AppendStartupTrace(L"InitInstance: TextureServer created");

  LoadOriginalDll();
  AppendStartupTrace(L"InitInstance: LoadOriginalDll ok");

  // we detour the original Direct3DCreate9 to our MyDirect3DCreate9
  Direct3DCreate9_fn = (Direct3DCreate9_type) GetProcAddress(gl_hOriginalDll, "Direct3DCreate9");
  if (Direct3DCreate9_fn!=NULL)
  {
    Message("Detour: Direct3DCreate9\n");
    Direct3DCreate9_fn = (Direct3DCreate9_type)DetourFunc( (BYTE*)Direct3DCreate9_fn, (BYTE*)uMod_Direct3DCreate9, 5);
    AppendStartupTrace(L"InitInstance: Detour Direct3DCreate9 ok");
  }

  Direct3DCreate9Ex_fn = (Direct3DCreate9Ex_type) GetProcAddress(gl_hOriginalDll, "Direct3DCreate9Ex");
  if (Direct3DCreate9Ex_fn!=NULL)
  {
    Message("Detour: Direct3DCreate9Ex\n");
    Direct3DCreate9Ex_fn = (Direct3DCreate9Ex_type)DetourFunc( (BYTE*)Direct3DCreate9Ex_fn, (BYTE*)uMod_Direct3DCreate9Ex, 7);
    AppendStartupTrace(L"InitInstance: Detour Direct3DCreate9Ex ok");
  }

  if (gl_TextureServer->OpenPipe(game)) //open the pipe and send the name+path of this executable
  {
    DWORD error = GetLastError();
    Message("InitInstance: Pipe not opened (error %lu)\n", error);
    AppendStartupTrace(L"InitInstance: OpenPipe failed");
    return;
  }
  AppendStartupTrace(L"InitInstance: OpenPipe ok");

  gl_ServerThread = CreateThread( NULL, 0, ServerThread, NULL, 0, NULL); //creating a thread for the mainloop
  if (gl_ServerThread==NULL) {Message("InitInstance: Serverthread not started\n");}
  AppendStartupTrace(L"InitInstance: ServerThread started");
}

void LoadOriginalDll(void)
{
  char buffer[MAX_PATH];
  GetSystemDirectory(buffer,MAX_PATH); //get the system directory, we need to open the original d3d9.dll

  // Append dll name
  strcat_s( buffer, MAX_PATH,"\\d3d9.dll");

  // try to load the system's d3d9.dll, if pointer empty
  if (!gl_hOriginalDll) gl_hOriginalDll = LoadLibrary(buffer);

  if (!gl_hOriginalDll)
  {
    ExitProcess(0); // exit the hard way
  }
}

void ExitInstance()
{
  if (gl_TextureServer!=NULL)
  {
    gl_TextureServer->ClosePipe(); //This must be done before the server thread is killed, because the server thread will endless wait on the ReadFile()
  }
  if (gl_ServerThread!=NULL)
  {
    CloseHandle(gl_ServerThread); // kill the server thread
    gl_ServerThread = NULL;
  }
  if (gl_TextureServer!=NULL)
  {
    delete gl_TextureServer; //delete the texture server
    gl_TextureServer = NULL;
  }

  // Release the system's d3d9.dll
  if (gl_hOriginalDll!=NULL)
  {
    FreeLibrary(gl_hOriginalDll);
    gl_hOriginalDll = NULL;
  }

  CloseMessage();
}

/*
 * We inject the dll into the game, thus we retour the original Direct3DCreate9 function to our MyDirect3DCreate9 function
 */

IDirect3D9 *APIENTRY uMod_Direct3DCreate9(UINT SDKVersion)
{
  Message("uMod_Direct3DCreate9:  original %lu, uMod %lu\n", Direct3DCreate9_fn, uMod_Direct3DCreate9);

  // in the Internet are many tutorials for detouring functions and all of them will work without the following 5 marked lines
  // but somehow, for me it only works, if I retour the function and calling afterward the original function

  // BEGIN

  LoadOriginalDll();

  RetourFunc((BYTE*) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9"), (BYTE*)Direct3DCreate9_fn, 5);
  Direct3DCreate9_fn = (Direct3DCreate9_type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9");

/*
  if (Direct3DCreate9Ex_fn!=NULL)
  {
    RetourFunc((BYTE*) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9Ex"), (BYTE*)Direct3DCreate9Ex_fn, 7);
    Direct3DCreate9Ex_fn = (Direct3DCreate9Ex_type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9Ex");
  }
  */
  // END

  IDirect3D9 *pIDirect3D9_orig = NULL;
  if (Direct3DCreate9_fn)
  {
    pIDirect3D9_orig = Direct3DCreate9_fn(SDKVersion); //creating the original IDirect3D9 object
  }
  else return (NULL);
  uMod_IDirect3D9 *pIDirect3D9;
  if (pIDirect3D9_orig)
  {
    pIDirect3D9 = new uMod_IDirect3D9( pIDirect3D9_orig, gl_TextureServer); //creating our uMod_IDirect3D9 object
  }

  // we detour again
  Direct3DCreate9_fn = (Direct3DCreate9_type)DetourFunc( (BYTE*) Direct3DCreate9_fn, (BYTE*)uMod_Direct3DCreate9,5);
  /*
  if (Direct3DCreate9Ex_fn!=NULL)
  {
    Direct3DCreate9Ex_fn = (Direct3DCreate9Ex_type)DetourFunc( (BYTE*) Direct3DCreate9Ex_fn, (BYTE*)uMod_Direct3DCreate9Ex,7);
  }
*/
  return (pIDirect3D9); //return our object instead of the "real one"
}

HRESULT APIENTRY uMod_Direct3DCreate9Ex( UINT SDKVersion, IDirect3D9Ex **ppD3D)
{
  Message( "uMod_Direct3DCreate9Ex:  original %lu, uMod %lu\n", Direct3DCreate9Ex_fn, uMod_Direct3DCreate9Ex);

  // in the Internet are many tutorials for detouring functions and all of them will work without the following 5 marked lines
  // but somehow, for me it only works, if I retour the function and calling afterward the original function

  // BEGIN

  LoadOriginalDll();
  /*
  if (Direct3DCreate9_fn!=NULL)
  {
    RetourFunc((BYTE*) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9"), (BYTE*)Direct3DCreate9_fn, 5);
    Direct3DCreate9_fn = (Direct3DCreate9_type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9");
  }
*/
  RetourFunc((BYTE*) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9Ex"), (BYTE*)Direct3DCreate9Ex_fn, 7);
  Direct3DCreate9Ex_fn = (Direct3DCreate9Ex_type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9Ex");
  // END

  IDirect3D9Ex *pIDirect3D9Ex_orig = NULL;
  HRESULT ret;
  if (Direct3DCreate9Ex_fn)
  {
    ret = Direct3DCreate9Ex_fn(SDKVersion, &pIDirect3D9Ex_orig); //creating the original IDirect3D9 object
  }
  else return (D3DERR_NOTAVAILABLE);

  if (ret!=S_OK) return (ret);

  uMod_IDirect3D9Ex *pIDirect3D9Ex;
  if (pIDirect3D9Ex_orig)
  {
    pIDirect3D9Ex = new uMod_IDirect3D9Ex( pIDirect3D9Ex_orig, gl_TextureServer); //creating our uMod_IDirect3D9 object
  }

  // we detour again
/*
  if (Direct3DCreate9_fn!=NULL)
  {
    Direct3DCreate9_fn = (Direct3DCreate9_type)DetourFunc( (BYTE*) Direct3DCreate9_fn, (BYTE*)uMod_Direct3DCreate9,5);
  }
  */
  Direct3DCreate9Ex_fn = (Direct3DCreate9Ex_type)DetourFunc( (BYTE*) Direct3DCreate9Ex_fn, (BYTE*)uMod_Direct3DCreate9Ex,7);
  ppD3D = (IDirect3D9Ex**) &pIDirect3D9Ex; //return our object instead of the "real one"
  return (ret);
}

bool HookThisProgram( wchar_t *ret)
{
  wchar_t Executable[MAX_PATH];
  GetModuleFileNameW( GetModuleHandle( NULL ), Executable, MAX_PATH ); //ask for name and path of this executable

  // we inject directly
  int i=0;
  while ( Executable[i]) {ret[i]=Executable[i]; i++;}
  ret[i]=0;
  return true;
}

void *DetourFunc(BYTE *src, const BYTE *dst, const int len)
{
  BYTE *jmp = (BYTE*)malloc(len+5);
  DWORD dwback = 0;
  VirtualProtect(jmp, len+5, PAGE_EXECUTE_READWRITE, &dwback); //This is the addition needed for Windows 7 RC
  VirtualProtect(src, len, PAGE_READWRITE, &dwback);
  memcpy(jmp, src, len);    jmp += len;
  jmp[0] = 0xE9;
  *(DWORD*)(jmp+1) = (DWORD)(src+len - jmp) - 5;
  memset(src, 0x90, len);
  src[0] = 0xE9;
  *(DWORD*)(src+1) = (DWORD)(dst - src) - 5;
  VirtualProtect(src, len, dwback, &dwback);
  return (jmp-len);
}

bool RetourFunc(BYTE *src, BYTE *restore, const int len)
{
  DWORD dwback;
  if(!VirtualProtect(src, len, PAGE_READWRITE, &dwback))  { return (false); }
  if(!memcpy(src, restore, len))              { return (false); }
  restore[0] = 0xE9;
  *(DWORD*)(restore+1) = (DWORD)(src - restore) - 5;
  if(!VirtualProtect(src, len, dwback, &dwback))      { return (false); }
  return (true);
}
