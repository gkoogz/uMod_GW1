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





#include "uMod_Main.h"

namespace {
struct RelaunchInfo
{
  wxString exe_path;
  wxString command_line;
  wxString dll_path;
  HANDLE process;
};

static wxString GetInjectedGamesPath(void)
{
  return GetReforgedAppDataPath("uMod_Reforged_DI_Games.txt");
}

static wxString GetInjectedDllPath(void)
{
  return GetReforgedAppDataPath(uMod_d3d9_DI_dll);
}

static bool ExtractEmbeddedDll(const wxString &destination)
{
  HMODULE module = GetModuleHandle(NULL);
  HRSRC resource = FindResource(module, MAKEINTRESOURCE(IDR_UMOD_REFORGED_DLL), RT_RCDATA);
  if (resource == NULL) return false;
  HGLOBAL resource_handle = LoadResource(module, resource);
  if (resource_handle == NULL) return false;
  DWORD resource_size = SizeofResource(module, resource);
  if (resource_size == 0) return false;
  void *resource_data = LockResource(resource_handle);
  if (resource_data == NULL) return false;

  wxFile file;
  if (!file.Open(destination, wxFile::write)) return false;
  file.Write(resource_data, resource_size);
  file.Close();
  return true;
}

static bool EnsureInjectedDllAvailable(wxString &dll_path)
{
  dll_path = GetInjectedDllPath();
  if (wxFile::Access(dll_path, wxFile::read)) return true;
  GetReforgedAppDataDir();
  return ExtractEmbeddedDll(dll_path);
}

static bool StartProcessWithInject(const wxString &game_path, const wxString &command_line, const wxString &dll_path, HANDLE &process)
{
  STARTUPINFOW si = {0};
  si.cb = sizeof(STARTUPINFO);
  PROCESS_INFORMATION pi = {0};

  wxString path = game_path.BeforeLast('\\');
  wxString exe;

  if (!command_line.IsEmpty()) exe << "\"" << game_path << "\" " << command_line;
  else exe = game_path;

  bool result = CreateProcess(NULL, (wchar_t*) exe.wc_str(), NULL, NULL, FALSE,
                              CREATE_SUSPENDED, NULL, path.wc_str(), &si, &pi);
  if (!result) return false;

  Inject(pi.hProcess, dll_path.wc_str(), "Nothing");
  ResumeThread(pi.hThread);
  CloseHandle(pi.hThread);
  process = pi.hProcess;
  return true;
}

static DWORD WINAPI RelaunchIfNeeded(LPVOID data)
{
  RelaunchInfo *info = reinterpret_cast<RelaunchInfo*>(data);
  if (info == NULL) return 0;

  DWORD wait = WaitForSingleObject(info->process, 3000);
  CloseHandle(info->process);
  info->process = INVALID_HANDLE_VALUE;

  if (wait == WAIT_OBJECT_0)
  {
    HANDLE proc = INVALID_HANDLE_VALUE;
    if (StartProcessWithInject(info->exe_path, info->command_line, info->dll_path, proc))
    {
      if (proc != INVALID_HANDLE_VALUE) CloseHandle(proc);
    }
  }

  delete info;
  return 0;
}
}



#ifndef __CDT_PARSER__

DEFINE_EVENT_TYPE(uMod_EVENT_TYPE)

BEGIN_EVENT_TABLE(uMod_Frame, wxFrame)
  EVT_CLOSE(uMod_Frame::OnClose)

  EVT_BUTTON(ID_Button_Open, uMod_Frame::OnButtonOpen)
  EVT_BUTTON(ID_Button_Path, uMod_Frame::OnButtonPath)
  EVT_BUTTON(ID_Button_Update, uMod_Frame::OnButtonUpdate)
  EVT_BUTTON(ID_Button_Reload, uMod_Frame::OnButtonReload)

  EVT_COMMAND  (ID_Add_Game, uMod_EVENT_TYPE, uMod_Frame::OnAddGame)
  EVT_COMMAND  (ID_Delete_Game, uMod_EVENT_TYPE, uMod_Frame::OnDeleteGame)
END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)
#endif

MyApp::~MyApp(void)
{
  if (CheckForSingleRun!=NULL) CloseHandle( CheckForSingleRun);
}


bool MyApp::OnInit(void)
{
  SetAppName("uMod_Reforged");
  uMod_Settings set;
  set.Load();

  Language = new uMod_Language(set.Language);
  CheckForSingleRun = CreateMutex( NULL, true, L"Global\\uMod_Reforged_CheckForSingleRun");
  if (ERROR_ALREADY_EXISTS == GetLastError())
  {
    wxMessageBox( Language->Error_AlreadyRunning, "ERROR", wxOK|wxICON_ERROR);
    return false;
  }
  uMod_Frame *frame = new uMod_Frame( uMod_VERSION, set);
  SetTopWindow( frame );

  return true;
}


uMod_Frame::uMod_Frame(const wxString& title, uMod_Settings &set)
       : wxFrame((wxFrame *)NULL, -1, title, wxPoint(set.XPos,set.YPos), wxSize(set.XSize,set.YSize)), Settings(set)
{
  SetIcon(wxICON(MAINICON));

  Server = new uMod_Server( this);
  Server->Create();
  Server->Run();

  MainSizer = new wxBoxSizer(wxVERTICAL);

  ActivePipe.In = INVALID_HANDLE_VALUE;
  ActivePipe.Out = INVALID_HANDLE_VALUE;
  GamePage = new uMod_GamePage( this, "", "", ActivePipe, this);
  MainSizer->Add( (wxWindow*) GamePage, 1, wxEXPAND , 0 );


  SetSizer( MainSizer);

  NumberOfGames = 0;
  MaxNumberOfGames  = 10;
  Clients = NULL;
  if (GetMemory( Clients, MaxNumberOfGames))
  {
    wxMessageBox( Language->Error_Memory, "ERROR", wxOK|wxICON_ERROR);
  }
  GamePage->LoadLauncherSettings();

  Show( true );

  {
    HMODULE dll = LoadLibraryW( L"D3DX9_43.dll");
    if (dll==NULL)
    {
      wxMessageBox( Language->Error_D3DX9NotFound, "ERROR", wxOK|wxICON_ERROR);
    }
    else FreeLibrary(dll);
  }

  DeactivateGamesControl();
}

uMod_Frame::~uMod_Frame(void)
{
  if (Server!=(uMod_Server*)0)
  {
    KillServer();
    Server->Wait();
    delete Server;
    Server = NULL;
  }

  if (Clients!=NULL) delete [] Clients;

  Settings.Language = Language->GetCurrentLanguage();
  GetSize( &Settings.XSize, &Settings.YSize);
  GetPosition( &Settings.XPos, &Settings.YPos);
  Settings.Save();
}

bool uMod_Frame::IsGameActive(void) const
{
  return ActivePipe.Out != INVALID_HANDLE_VALUE;
}

int uMod_Frame::KillServer(void)
{
  DWORD start = GetTickCount();
  while (!WaitNamedPipe(PIPE_Game2uMod, 200))
  {
    if (GetLastError() != ERROR_SEM_TIMEOUT) return -1;
    if (GetTickCount() - start > 2000) return -1;
  }
  HANDLE pipe = CreateFileW( PIPE_Game2uMod,// pipe name
                 GENERIC_WRITE,
                 0,              // no sharing
                 NULL,           // default security attributes
                 OPEN_EXISTING,  // opens existing pipe
                 0,              // default attributes
                 NULL);          // no template file

  if (pipe == INVALID_HANDLE_VALUE) return -1;

  const wchar_t *str = ABORT_SERVER;
  unsigned int len=0u;
  while (str[len]) len++;
  len++; //to send also the zero
  unsigned long num;
  WriteFile( pipe, (const void*) str, len*sizeof(wchar_t), &num, NULL);
  CloseHandle(pipe);
  return 0;
}



void uMod_Frame::OnAddGame( wxCommandEvent &event)
{
  if (NumberOfGames>=MaxNumberOfGames)
  {
    if (GetMoreMemory( Clients, MaxNumberOfGames, MaxNumberOfGames+10))
    {
      wxMessageBox( Language->Error_Memory, "ERROR", wxOK|wxICON_ERROR);
      return;
    }
    MaxNumberOfGames += 10;
  }

  wxString name = ((uMod_Event&)event).GetName();
  PipeStruct pipe;

  pipe.In = ((uMod_Event&)event).GetPipeIn();
  pipe.Out = ((uMod_Event&)event).GetPipeOut();

  uMod_Client *client = new uMod_Client( pipe, this);
  client->Create();
  client->Run();

  ActivePipe.In = client->Pipe.In;
  ActivePipe.Out = client->Pipe.Out;
  GamePage->SetGameInfo( name, "");
  GamePage->ResetConnection();
  if (GamePage->ReloadGame())
  {
    wxMessageBox(GamePage->LastError, "ERROR", wxOK|wxICON_ERROR);
    GamePage->LastError.Empty();
  }
  if (GamePage->LastError.Len()>0)
  {
    wxMessageBox(GamePage->LastError, "ERROR", wxOK|wxICON_ERROR);
    GamePage->LastError.Empty();
  }

  Clients[NumberOfGames] = client;
  NumberOfGames++;
  if (NumberOfGames==1) ActivateGamesControl();
}

void uMod_Frame::OnDeleteGame( wxCommandEvent &event)
{
  uMod_Client *client = ((uMod_Event&)event).GetClient();
  for (int i=0; i<NumberOfGames; i++) if (Clients[i]==client)
  {
    ActivePipe.In = INVALID_HANDLE_VALUE;
    ActivePipe.Out = INVALID_HANDLE_VALUE;
    Clients[i]->Wait();
    delete Clients[i];
    NumberOfGames--;
    for (int j=i; j<NumberOfGames; j++) Clients[j] = Clients[j+1];

    if (NumberOfGames==0) DeactivateGamesControl();
    return;
  }
}


void uMod_Frame::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto() && NumberOfGames>0)
  {
    if (wxMessageBox(Language->ExitGameAnyway, "ERROR", wxYES_NO|wxICON_ERROR)!=wxYES) {event.Veto(); return;}
  }
  if (Clients!=NULL)
  {
    for (int i=0; i<NumberOfGames; i++)
    {
      if (Clients[i]==NULL) continue;
      if (Clients[i]->Pipe.In != INVALID_HANDLE_VALUE)
      {
        CloseHandle(Clients[i]->Pipe.In);
        Clients[i]->Pipe.In = INVALID_HANDLE_VALUE;
      }
      if (Clients[i]->Pipe.Out != INVALID_HANDLE_VALUE)
      {
        CloseHandle(Clients[i]->Pipe.Out);
        Clients[i]->Pipe.Out = INVALID_HANDLE_VALUE;
      }
      Clients[i]->Delete();
      Clients[i] = NULL;
    }
    NumberOfGames = 0;
  }
  if (Server!=NULL)
  {
    if (KillServer() == 0)
    {
      Server->Wait();
    }
    delete Server;
    Server = NULL;
  }
  Destroy();
}

void uMod_Frame::OnButtonOpen(wxCommandEvent& WXUNUSED(event))
{
  if (GamePage==NULL) return;


  //wxString file_name = wxFileSelector( Language->ChooseFile, page->GetOpenPath(), "", "*.*",  "textures (*.dds)|*.dds|zip (*.zip)|*.zip|tpf (*.tpf)|*.tpf", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  wxString file_name = wxFileSelector( Language->ChooseFile, GamePage->GetOpenPath(), "", "",  "", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  if ( !file_name.empty() )
  {
    GamePage->SetOpenPath(file_name.BeforeLast( '/'));
    if (GamePage->AddTexture( file_name))
    {
      wxMessageBox(GamePage->LastError, "ERROR", wxOK|wxICON_ERROR);
      GamePage->LastError.Empty();
    }
  }
}

void uMod_Frame::OnButtonPath(wxCommandEvent& WXUNUSED(event))
{
  if (GamePage==NULL) return;

  wxString dir = wxDirSelector( Language->ChooseDir, GamePage->GetSavePath());
  if ( !dir.empty() )
  {
    GamePage->SetSavePath( dir);
  }
}

void uMod_Frame::OnButtonUpdate(wxCommandEvent& WXUNUSED(event))
{
  if (GamePage==NULL) return;
  if (GamePage->UpdateGame())
  {
    wxMessageBox(GamePage->LastError, "ERROR", wxOK|wxICON_ERROR);
    GamePage->LastError.Empty();
  }
}

void uMod_Frame::OnButtonReload(wxCommandEvent& WXUNUSED(event))
{
  if (GamePage==NULL) return;
  if (GamePage->ReloadGame())
  {
    wxMessageBox(GamePage->LastError, "ERROR", wxOK|wxICON_ERROR);
    GamePage->LastError.Empty();
  }
}




int uMod_Frame::LaunchGame(const wxString &game_path, const wxString &command_line)
{
  if (game_path.IsEmpty()) return -1;
  wxString dll;
  if (!EnsureInjectedDllAvailable(dll))
  {
    wxString message = Language->Error_FileOpen;
    message << "\n" << GetInjectedDllPath();
    wxMessageBox( message, "ERROR",  wxOK|wxICON_ERROR);
    return -1;
  }

  HANDLE process = INVALID_HANDLE_VALUE;
  if (!StartProcessWithInject(game_path, command_line, dll, process))
  {
    wxMessageBox( Language->Error_ProcessNotStarted, "ERROR",  wxOK|wxICON_ERROR);
    return -1;
  }

  RelaunchInfo *info = new RelaunchInfo;
  info->exe_path = game_path;
  info->command_line = command_line;
  info->dll_path = dll;
  info->process = process;
  HANDLE thread = CreateThread(NULL, 0, RelaunchIfNeeded, info, 0, NULL);
  if (thread != NULL) CloseHandle(thread);
  else
  {
    if (process != INVALID_HANDLE_VALUE) CloseHandle(process);
    delete info;
  }
  return 0;
}



int uMod_Frame::ActivateGamesControl(void)
{
  if (GamePage!=NULL) GamePage->EnableGameControls( true);

  return 0;
}

int uMod_Frame::DeactivateGamesControl(void)
{
  if (GamePage!=NULL) GamePage->EnableGameControls( false);
  return 0;
}

int uMod_Frame::GetInjectedGames( wxArrayString &games, wxArrayString &cmd)
{
  wxFile file;

  wxString injected_games_path = GetInjectedGamesPath();
  if (!file.Access( injected_games_path, wxFile::read)) {return 0;}
  file.Open( injected_games_path, wxFile::read);
  if (!file.IsOpened()) {return -1;}

  unsigned len = file.Length();
  if (len == 0 || (len % 2) != 0) {file.Close(); wxRemoveFile(injected_games_path); return 0;}

  unsigned char* buffer;
  try {buffer = new unsigned char [len+2];}
  catch (...) {LastError << Language->Error_Memory; return -1;}

  unsigned int result = file.Read( buffer, len);
  file.Close();

  if (result != len) {delete [] buffer; wxRemoveFile(injected_games_path); return -1;}

  wchar_t *buff = (wchar_t*)buffer;
  len/=2;
  buff[len]=0;

  wxString content;
  content =  buff;
  delete [] buffer;

  wxStringTokenizer token( content, "\n");

  int num = token.CountTokens();

  games.Empty();
  games.Alloc(num);
  cmd.Empty();
  cmd.Alloc(num);
  wxString entry;

  for (int i=0; i<num; i++)
  {
    entry = token.GetNextToken();
    games.Add( entry.BeforeFirst('|'));
    cmd.Add( entry.AfterFirst('|'));
  }
  return 0;
}

int uMod_Frame::SetInjectedGames( wxArrayString &games, wxArrayString &cmd)
{
  wxFile file;

  wxString injected_games_path = GetInjectedGamesPath();
  file.Open( injected_games_path, wxFile::write);
  if (!file.IsOpened()) {LastError << Language->Error_FileOpen << "\n" << injected_games_path ; return -1;}
  wxString content;

  int num = games.GetCount();
  for (int i=0; i<num; i++)
  {
    content = games[i];
    content << "|" <<  cmd[i] << "\n";
    file.Write( content.wc_str(), content.Len()*2);
  }
  file.Close();
  return 0;
}
