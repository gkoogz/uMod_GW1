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
#include <wx/filename.h>
#include <wx/stdpaths.h>





#ifndef __CDT_PARSER__

DEFINE_EVENT_TYPE(uMod_EVENT_TYPE)

BEGIN_EVENT_TABLE(uMod_Frame, wxFrame)
  EVT_CLOSE(uMod_Frame::OnClose)

  EVT_BUTTON(ID_Button_Open, uMod_Frame::OnButtonOpen)
  EVT_BUTTON(ID_Button_Path, uMod_Frame::OnButtonPath)
  EVT_BUTTON(ID_Button_Update, uMod_Frame::OnButtonUpdate)
  EVT_BUTTON(ID_Button_Reload, uMod_Frame::OnButtonReload)
  EVT_BUTTON(ID_Button_Launch, uMod_Frame::OnButtonLaunch)
  EVT_BUTTON(ID_Button_Locate, uMod_Frame::OnButtonLocate)
  EVT_BUTTON(ID_Button_CommandLineHelp, uMod_Frame::OnCommandLineHelp)

  EVT_MENU(ID_Menu_Help, uMod_Frame::OnMenuHelp)
  EVT_MENU(ID_Menu_About, uMod_Frame::OnMenuAbout)
  EVT_MENU(ID_Menu_Acknowledgement, uMod_Frame::OnMenuAcknowledgement)


  EVT_MENU(ID_Menu_LoadTemplate, uMod_Frame::OnMenuOpenTemplate)
  EVT_MENU(ID_Menu_SaveTemplate, uMod_Frame::OnMenuSaveTemplate)
  EVT_MENU(ID_Menu_SaveTemplateAs, uMod_Frame::OnMenuSaveTemplateAs)
  EVT_MENU(ID_Menu_SetDefaultTemplate, uMod_Frame::OnMenuSetDefaultTemplate)

  EVT_MENU(ID_Menu_Lang, uMod_Frame::OnMenuLanguage)
  EVT_MENU(ID_Menu_Exit, uMod_Frame::OnMenuExit)

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
  uMod_Settings set;
  set.Load();

  Language = new uMod_Language(set.Language);
  CheckForSingleRun = CreateMutex( NULL, true, L"Global\\uMod_CheckForSingleRun");
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

  MenuBar = new wxMenuBar;
  //MenuMain = new wxMenu;
  MenuMain = new wxMenu;
  MenuHelp = new wxMenu;

  MenuMain->Append( ID_Menu_LoadTemplate, Language->MenuLoadTemplate );
  MenuMain->Append( ID_Menu_SaveTemplate, Language->MenuSaveTemplate );
  MenuMain->Append( ID_Menu_SaveTemplateAs, Language->MenuSaveTemplateAs );
  MenuMain->Append( ID_Menu_SetDefaultTemplate, Language->MenuSetDefaultTemplate );
  MenuMain->AppendSeparator();
  MenuMain->Append( ID_Menu_Lang, Language->MenuLanguage );
  MenuMain->Append( ID_Menu_Exit, Language->MenuExit );

  MenuHelp->Append( ID_Menu_Help, Language->MenuHelp );
  MenuHelp->Append( ID_Menu_About, Language->MenuAbout );
  MenuHelp->Append( ID_Menu_Acknowledgement, Language->MenuAcknowledgement );

  MenuBar->Append( MenuMain, Language->MainMenuMain );
  MenuBar->Append( MenuHelp, Language->MainMenuHelp );

  SetMenuBar(MenuBar);


  MainSizer = new wxBoxSizer(wxVERTICAL);

  Notebook = new wxNotebook( this, wxID_ANY);
  Notebook->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  MainSizer->Add( (wxWindow*) Notebook, 1, wxEXPAND , 0 );

  LauncherPanel = new wxPanel( Notebook, wxID_ANY);
  wxBoxSizer *launcherSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *launchSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *commandSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *commandLabelSizer = new wxBoxSizer(wxHORIZONTAL);
  LaunchButton = new wxButton( LauncherPanel, ID_Button_Launch, Language->ButtonLaunchGuildWars, wxDefaultPosition, wxSize(220, 40));
  CommandLineLabel = new wxStaticText( LauncherPanel, wxID_ANY, Language->CommandLineLabel);
  CommandLineHelpButton = new wxButton( LauncherPanel, ID_Button_CommandLineHelp, "?", wxDefaultPosition, wxSize(24, 24));
  CommandLineHelpButton->SetToolTip( Language->CommandLineHelpTitle);
  CommandLineText = new wxTextCtrl( LauncherPanel, wxID_ANY, "", wxDefaultPosition, wxSize(300, 24));
  CommandLineText->SetToolTip( Language->CommandLine);
  CommandLineHelpButton->Bind(wxEVT_ENTER_WINDOW, &uMod_Frame::OnCommandLineHelpHover, this);
  commandLabelSizer->Add( (wxWindow*) CommandLineLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 6);
  commandLabelSizer->Add( (wxWindow*) CommandLineHelpButton, 0, wxALIGN_CENTER_VERTICAL, 0);
  commandSizer->Add( commandLabelSizer, 0, wxBOTTOM, 4);
  commandSizer->Add( (wxWindow*) CommandLineText, 0, wxEXPAND, 0);
  launchSizer->Add( (wxWindow*) LaunchButton, 0, wxEXPAND|wxRIGHT, 10);
  launchSizer->Add( commandSizer, 1, wxEXPAND, 0);
  launcherSizer->Add( launchSizer, 0, wxEXPAND|wxALL, 10);

  wxBoxSizer *locateSizer = new wxBoxSizer(wxHORIZONTAL);
  LocateButton = new wxButton( LauncherPanel, ID_Button_Locate, Language->ButtonLocateGuildWars, wxDefaultPosition, wxSize(220, 30));
  ExePathText = new wxTextCtrl( LauncherPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  locateSizer->Add( (wxWindow*) LocateButton, 0, wxEXPAND|wxRIGHT, 10);
  locateSizer->Add( (wxWindow*) ExePathText, 1, wxEXPAND, 0);
  launcherSizer->Add( locateSizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);

  LauncherPanel->SetSizer( launcherSizer);
  Notebook->AddPage( LauncherPanel, Language->TabLauncher, true);

  DefaultPipe.In = INVALID_HANDLE_VALUE;
  DefaultPipe.Out = INVALID_HANDLE_VALUE;
  TextureRipperPage = new uMod_GamePage( Notebook, "", "", &DefaultPipe);
  Notebook->AddPage( TextureRipperPage, Language->TabTextureRipper, false);

  ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  OpenButton = new wxButton( this, ID_Button_Open, Language->ButtonOpen, wxDefaultPosition, wxSize(100,24));
  DirectoryButton = new wxButton( this, ID_Button_Path, Language->ButtonDirectory, wxDefaultPosition, wxSize(100,24));
  UpdateButton = new wxButton( this, ID_Button_Update, Language->ButtonUpdate, wxDefaultPosition, wxSize(100,24));
  ReloadButton = new wxButton( this, ID_Button_Reload, Language->ButtonReload, wxDefaultPosition, wxSize(100,24));

  ButtonSizer->Add( (wxWindow*) OpenButton, 1, wxEXPAND, 0);
  ButtonSizer->Add( (wxWindow*) DirectoryButton, 1, wxEXPAND, 0);
  ButtonSizer->Add( (wxWindow*) UpdateButton, 1, wxEXPAND, 0);
  ButtonSizer->Add( (wxWindow*) ReloadButton, 1, wxEXPAND, 0);
  MainSizer->Add( ButtonSizer, 0, wxEXPAND , 0 );


  SetSizer( MainSizer);

  NumberOfGames = 0;
  MaxNumberOfGames  = 10;
  Clients = NULL;
  if (GetMemory( Clients, MaxNumberOfGames))
  {
    wxMessageBox( Language->Error_Memory, "ERROR", wxOK|wxICON_ERROR);
  }
  LoadTemplate();
  LoadLauncherSettings();
  UpdateLaunchButtonState();

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

int uMod_Frame::KillServer(void)
{
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

  wxString save_file;
  int num = SaveFile_Exe.GetCount();
  for (int i=0; i<num; i++) if (name==SaveFile_Exe[i])
  {
    save_file = SaveFile_Name[i];
    break;
  }

  if (TextureRipperPage!=NULL)
  {
    TextureRipperPage->SetPipe( &client->Pipe, name);
    if (save_file.Len()>0)
    {
      if (TextureRipperPage->LoadTemplate(save_file))
      {
        wxMessageBox(TextureRipperPage->LastError, "ERROR", wxOK|wxICON_ERROR);
        TextureRipperPage->LastError.Empty();
      }
    }
    Notebook->SetSelection(1);
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
    if (TextureRipperPage!=NULL)
    {
      TextureRipperPage->SetPipe( &DefaultPipe, TextureRipperPage->GetExeName());
    }
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
  SaveLauncherSettings( ExePathText->GetValue(), CommandLineText->GetValue());
  event.Skip();
  Destroy();
}

void uMod_Frame::OnButtonOpen(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;


  //wxString file_name = wxFileSelector( Language->ChooseFile, page->GetOpenPath(), "", "*.*",  "textures (*.dds)|*.dds|zip (*.zip)|*.zip|tpf (*.tpf)|*.tpf", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  wxString file_name = wxFileSelector( Language->ChooseFile, page->GetOpenPath(), "", "",  "", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  if ( !file_name.empty() )
  {
    page->SetOpenPath(file_name.BeforeLast( '/'));
    if (page->AddTexture( file_name))
    {
      wxMessageBox(page->LastError, "ERROR", wxOK|wxICON_ERROR);
      page->LastError.Empty();
    }
  }
}

void uMod_Frame::OnButtonPath(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;

  wxString dir = wxDirSelector( Language->ChooseDir, page->GetSavePath());
  if ( !dir.empty() )
  {
    page->SetSavePath( dir);
  }
}

void uMod_Frame::OnButtonUpdate(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;
  if (page->UpdateGame())
  {
    wxMessageBox(page->LastError, "ERROR", wxOK|wxICON_ERROR);
    page->LastError.Empty();
  }
}

void uMod_Frame::OnButtonReload(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;
  if (page->ReloadGame())
  {
    wxMessageBox(page->LastError, "ERROR", wxOK|wxICON_ERROR);
    page->LastError.Empty();
  }
}




void uMod_Frame::OnMenuOpenTemplate(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;


  //wxString file_name = wxFileSelector( Language->ChooseFile, page->GetOpenPath(), "", "*.*",  "textures (*.dds)|*.dds|zip (*.zip)|*.zip|tpf (*.tpf)|*.tpf", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

  wxString dir = wxGetCwd();
  dir << "/templates";
  wxString file_name = wxFileSelector( Language->ChooseFile, dir, "", "*.txt",  "text (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  if ( !file_name.empty() )
  {
    if (page->LoadTemplate( file_name))
    {
      wxMessageBox(page->LastError, "ERROR", wxOK|wxICON_ERROR);
      page->LastError.Empty();
    }
  }
}

void uMod_Frame::OnMenuSaveTemplate(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;

  wxString file_name = page->GetTemplateName();

  if ( file_name.empty() )
  {
    wxString dir = wxGetCwd();
    dir << "/templates";
    file_name = wxFileSelector( Language->ChooseFile, dir, "", "*.txt",  "text (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this);
  }
  if ( !file_name.empty() )
  {
    if (page->SaveTemplate(file_name))
    {
      wxMessageBox(page->LastError, "ERROR", wxOK|wxICON_ERROR);
      page->LastError.Empty();
    }
  }
}

void uMod_Frame::OnMenuSaveTemplateAs(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;


  wxString dir = wxGetCwd();
  dir << "/templates";
  wxString file_name = wxFileSelector( Language->ChooseFile, dir, "", "*.txt",  "text (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this);
  if ( !file_name.empty() )
  {
    if (page->SaveTemplate(file_name))
    {
      wxMessageBox(page->LastError, "ERROR", wxOK|wxICON_ERROR);
      page->LastError.Empty();
    }
  }
}

void uMod_Frame::OnMenuSetDefaultTemplate(wxCommandEvent& WXUNUSED(event))
{
  uMod_GamePage *page = TextureRipperPage;
  if (page==NULL) return;

  wxString exe = page->GetExeName();
  wxString file = page->GetTemplateName();

  int num = SaveFile_Exe.GetCount();
  bool hit = false;
  for (int i=0; i<num; i++) if (SaveFile_Exe[i]==exe)
  {
    SaveFile_Name[i] = file;
    hit = true;
    break;
  }
  if (!hit)
  {
    SaveFile_Exe.Add(exe);
    SaveFile_Name.Add(file);
  }
  if (SaveTemplate())
  {
    wxMessageBox(LastError, "ERROR", wxOK|wxICON_ERROR);
    LastError.Empty();
  }
}

void uMod_Frame::OnMenuLanguage(wxCommandEvent& WXUNUSED(event))
{
  wxArrayString lang;
  Language->GetLanguages( lang);
  wxString choice = wxGetSingleChoice( Language->SelectLanguage, Language->SelectLanguage, lang);
  if (choice.Len()>0)
  {
    if (Language->LoadLanguage(choice))
    {
      wxMessageBox(Language->LastError, "ERROR", wxOK|wxICON_ERROR);
      Language->LastError.Empty();
      return;
    }
    MenuBar->SetMenuLabel( 0, Language->MainMenuMain);

    MenuMain->SetLabel( ID_Menu_LoadTemplate, Language->MenuLoadTemplate );
    MenuMain->SetLabel( ID_Menu_SaveTemplate, Language->MenuSaveTemplate );
    MenuMain->SetLabel( ID_Menu_SaveTemplateAs, Language->MenuSaveTemplateAs );
    MenuMain->SetLabel( ID_Menu_SetDefaultTemplate, Language->MenuSetDefaultTemplate );

    MenuMain->SetLabel( ID_Menu_Lang, Language->MenuLanguage);
    MenuMain->SetLabel( ID_Menu_Exit, Language->MenuExit );

    MenuBar->SetMenuLabel( 1, Language->MainMenuHelp);
    MenuHelp->SetLabel( ID_Menu_Help, Language->MenuHelp);
    MenuHelp->SetLabel( ID_Menu_About, Language->MenuAbout);
    MenuHelp->SetLabel( ID_Menu_Acknowledgement, Language->MenuAcknowledgement);


    OpenButton->SetLabel( Language->ButtonOpen);
    DirectoryButton->SetLabel( Language->ButtonDirectory);
    UpdateButton->SetLabel( Language->ButtonUpdate);
    ReloadButton->SetLabel( Language->ButtonReload);

    LaunchButton->SetLabel( Language->ButtonLaunchGuildWars);
    LocateButton->SetLabel( Language->ButtonLocateGuildWars);
    CommandLineText->SetToolTip( Language->CommandLine);
    CommandLineLabel->SetLabel( Language->CommandLineLabel);
    CommandLineHelpButton->SetToolTip( Language->CommandLineHelpTitle);
    if (Notebook->GetPageCount()>0) Notebook->SetPageText( 0, Language->TabLauncher);
    if (Notebook->GetPageCount()>1) Notebook->SetPageText( 1, Language->TabTextureRipper);
    if (TextureRipperPage!=NULL) TextureRipperPage->UpdateLanguage();
  }
}

void uMod_Frame::OnMenuExit(wxCommandEvent& WXUNUSED(event))
{
  Close();
}

void uMod_Frame::OnMenuHelp(wxCommandEvent& WXUNUSED(event))
{
  wxString help;
  if (Language->GetHelpMessage( help))
  {
    wxMessageBox(Language->LastError, "ERROR", wxOK|wxICON_ERROR);
    Language->LastError.Empty();
    return;
  }

  wxMessageBox( help, Language->MenuHelp, wxOK);
}

void uMod_Frame::OnMenuAbout(wxCommandEvent& WXUNUSED(event))
{
  wxString msg;
  msg << uMod_VERSION << "\n\nProject members:\n\nROTA (developer)\nKing Brace Blane (PR)\n\nhttp://code.google.com/p/texmod/";
  wxMessageBox( msg, "Info", wxOK);
}

void uMod_Frame::OnMenuAcknowledgement(wxCommandEvent& WXUNUSED(event))
{
  wxString msg;
  msg << "King Brace Blane and ROTA thank:\n\n";
  msg << "RS for coding the original TexMod and for information about the used hashing algorithm\n\n";
  msg << "EvilAlex for translation into Russian and bug fixing\n";
  msg << "ReRRemi for translation into French\n";
  msg << "mirHL for translation into Italian\n";
  msg << "Vergil for help with German ;)";

  wxMessageBox( msg, Language->MenuAcknowledgement, wxOK);
}

void uMod_Frame::OnButtonLaunch(wxCommandEvent& WXUNUSED(event))
{
  wxString exe_path = ExePathText->GetValue();
  exe_path.Trim();
  exe_path.Trim(false);
  if (exe_path.IsEmpty()) return;

  wxString command_line = CommandLineText->GetValue();
  SaveLauncherSettings( exe_path, command_line);
  if (TextureRipperPage!=NULL) TextureRipperPage->SetExeName(exe_path);

  STARTUPINFOW si = {0};
  si.cb = sizeof(STARTUPINFO);
  PROCESS_INFORMATION pi = {0};

  wxFileName exe_file(exe_path);
  wxString path = exe_file.GetPath();
  wxString exe;

  if (!command_line.IsEmpty()) exe << "\"" << exe_path << "\" " << command_line;
  else exe = exe_path;

  bool result = CreateProcess(NULL, (wchar_t*) exe.wc_str(), NULL, NULL, FALSE,
                              CREATE_SUSPENDED, NULL, path.wc_str(), &si, &pi);
  if(!result)
  {
    wxMessageBox( Language->Error_ProcessNotStarted, "ERROR",  wxOK|wxICON_ERROR);
    return ;
  }

  wxFileName exe_path_dll(wxStandardPaths::Get().GetExecutablePath());
  wxString dll = exe_path_dll.GetPath();
  dll.Append( L"\\" uMod_d3d9_DI_dll);

  Inject(pi.hProcess, dll.wc_str(), "Nothing");
  ResumeThread(pi.hThread);
}

void uMod_Frame::OnButtonLocate(wxCommandEvent& WXUNUSED(event))
{
  wxString file_name = wxFileSelector( Language->ChooseGame, "", "", "exe",  "binary (*.exe)|*.exe", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  if ( file_name.empty() ) return;

  ExePathText->SetValue( file_name);
  if (TextureRipperPage!=NULL) TextureRipperPage->SetExeName(file_name);
  SaveLauncherSettings( file_name, CommandLineText->GetValue());
  UpdateLaunchButtonState();
}

void uMod_Frame::OnCommandLineHelp(wxCommandEvent& WXUNUSED(event))
{
  ShowCommandLineHelp();
}

void uMod_Frame::OnCommandLineHelpHover(wxMouseEvent& event)
{
  ShowCommandLineHelp();
  event.Skip();
}

void uMod_Frame::ShowCommandLineHelp(void)
{
  wxString help;
  help <<
    "-autologin\n"
    "Use in conjunction with the Remember Account Name and Remember Password options on the login screen to go directly to the character select screen when launching the game.\n"
    "Note: You will be unable to log out while this option is set, attempting to log out from the character select screen will immediately log you back in.\n"
    "-bmp\n"
    "Forces the game to create screenshots as .BMP files. Use for creating high-quality screenshots at the expense of much larger files.\n"
    "-character \"<character-name-string>\"\n"
    "Automatically fills the client’s security question field with the specified name, first and last, as entered between the quotation marks. Enables auto-log-in if used in conjunction with -email and -password. If the name matches one of the characters on the account, it will be selected after logging in. Otherwise, no character will be selected after logging in.\n"
    "Note: This is still required despite character names' removal as a log-in requirement in the September 30th, 2020 Update. However, it does not need to actually match one of the character names on the account.\n"
    "-diag\n"
    "Creates, at startup, a detailed diagnostic logfile which contains data about connection tests and hardware information, located in your game directory (..\\Guild Wars\\NetworkDiag.log). Useful for determining what is wrong when Guild Wars is not functioning properly.\n"
    "Important: The program terminates after the logfile is created; since Guild Wars will not be able run with this parameter on, make sure to remove it once Guild Wars has finished creating the logfile.\n"
    "Note: It is also possible to run the Guild Wars diagnostic utility by clicking the \"Test My Computer\" link on the login screen.\n"
    "-dsound\n"
    "Forces the use of old DirectSound software mixer. Good for users whose computers are not compatible with the newer version.\n"
    "Also necessary for Creative ALchemy to work in Windows 7 and Vista.\n"
    "-dx8\n"
    "Useful for troubleshooting. Forces DirectX 8 compatibility; good for users whose computers are not compatible with the newer version. Using this switch with -noshaders may improve performance as well.\n"
    "Note: DirectX 8 support was removed on June 12, 2019, so this likely no longer has an effect.\n"
    "-email <string>\n"
    "Sets the e-mail address, as specified in <string>, on the log-in screen. Combine with -character and -password to create an auto-log-in shortcut for a specific account.\n"
    "-fps <number>\n"
    "Limits the frame rate to <number> frames per second, saving computer resources.\n"
    "The frame rate was limited to 90 frames per second by default on April 29, 2025, the -fps command line argument can now also be used to change this to a higher limit.\n"
    "-fqdn\n"
    "Tells the game to use `Auth1.0.ArenaNetworks.com` to connect instead of `Auth1.ArenaNetworks.com`.\n"
    "-image\n"
    "Forces the game to download all known updates.\n"
    "The program terminates after the update is completed; Guild Wars will not be able to run with this parameter on.\n"
    "Recommended: Create an extra Guild Wars shortcut (e.g. \"Guild Wars Updater\") specifically to use this command line switch.\n"
    "-image updates gw.dat; after updating, that file can be be transferred over to other machines.\n"
    "There can be up to 4 gigabytes of data to download. Once -image completes, the exact file size of gw.dat will vary across installations.\n"
    "-lodfull\n"
    "Instructs the client to use the highest level of \"LOD\" (level of detail). 3D assets will be rendered instead of 2D \"imposters\". There can still be some \"popping\" as some art assets move into and out of the edges of the view or a bump in the terrain. (Note: -lodfull was removed on May 9, 2018 and then re-added on May 16th, 2018).\n"
    "-log\n"
    "Enables logging of Guild Wars to the gw.log in the Guild Wars install directory.\n"
    "Note: This will delete the existing gw.log file and create a new one.\n"
    "-mce\n"
    "Start the client with Windows Media Center compatibility, switching the game to full screen and restarting Media Center (if available) after the client is closed.\n"
    "Note: This argument is added to the desktop shortcut by default in some cases.\n"
    "-mute\n"
    "Disables audio output, but still processes audio data internally (so for instance, lip syncing will still function).\n"
    "-newauth\n"
    "Was used to test the new authentication system.[1] This option was enabled by default in the February 22, 2012 update.\n"
    "-nopatchui\n"
    "Your client will patch invisibly in the background, until it has enough content to show the login screen, at which time the login window will appear on-screen.\n"
    "If combined with \"-image\", your client will fully patch, and then proceed to the login screen.\n"
    "This is similar to the option available in Guild Wars 2.\n"
    "-noshaders\n"
    "Troubleshooting option. Using this switch with -dx8 may improve performance as well.\n"
    "-nosound\n"
    "Disables audio system completely.\n"
    "-noui\n"
    "Disables the user interface, producing the same effect as when one pushes Ctrl + Shift + H in-game to toggle it.\n"
    "-oldauth\n"
    "Forces the client to use the old authentication system. (No longer works).\n"
    "-oldfov\n"
    "The old Field of View can be reenabled by using the new -oldfov command line argument (diagonal calculation) instead of the newer (vertical calculation).\n"
    "-password <string>\n"
    "Enables auto-log-in if <string> is the correct password for the account, and either (A) is used with -character and -email or (B) is used with -character and details have previously been saved using the \"Remember Account Name and Security Answer\" checkbox.\n"
    "Warning: Putting your password in a shortcut or a batch file makes it easily accessible to everybody with access to your computer. Never use this option if you are sharing your computer with other people whom you do not trust.\n"
    "Usage note: if your password contains spaces, put \" at the start and end of your password. Example: -password \"1234 5\" . These quotation symbols are not required if there are no spaces in your password. Example: -password 12345 .\n"
    "\n"
    "-perf\n"
    "Displays performance indicators. Triangles, frames per second and transfer rate in bytes per second are displayed in white text in the upper-right corner of the Guild Wars window.\n"
    "-prefresetlocal\n"
    "Resets all preferences that are stored locally in the game's .dat file.\n"
    "This command can be utilized to debug issues launching the game.\n"
    "-repair\n"
    "Attempts to fix the GW.dat file, in which all update information is stored.\n"
    "If installed by CD/DVD it will ask for the install disk (disk 1).\n"
    "Warning: If a lot of content has already been downloaded through updates, be prepared for a long wait as Guild Wars reconstructs the file.\n"
    "-resetmap\n"
    "Resets a characters map to the last accessible one they were in. Example: Some maps will crash the client, some will not load etc.\n"
    "If you attempt to load a character onto one of these, they will become stuck there for some time.\n"
    "Using the -resetmap switch will push them back to a valid map.\n"
    "-sndasio\n"
    "Useful for troubleshooting. Attempts to use an ASIO driver in software mode.\n"
    "-sndwinmm\n"
    "Attempts to use the Windows Multimedia audio driver in software mode.\n"
    "-stress <n>\n"
    "<n> is the time to run a stress test in milliseconds.\n"
    "The test will run for <n> milliseconds each frame.\n"
    "The stress test runs during loading screens and NOT while the game is running.\n"
    "-stress without a numeric argument is the same as -stress 0, which does no stress testing.\n"
    "-stress 10 is not particularly noticeable in terms of visibly slowing down the loading screen.\n"
    "-stress 100 will make your loading screen update at 10 fps as it spends 100 ms each frame doing the test.\n"
    "-stress 1000 will make your loading screen update at 1 fps.\n"
    "-stress 10000 will make your loading screen update at 0.1 fps.\n"
    "If the test fails, it will display a dialog box with this text: \"Guild Wars detected a possible hardware malfunction. This may be due to a faulty or overheating processor or memory chip. The application will now terminate.\" Your only choice is \"OK\".\n"
    "-uninstall\n"
    "Uninstalls Guild Wars.\n"
    "-update\n"
    "Prompts for install disk (disk 1) to update gw.dat\n"
    "-windowed\n"
    "Forces Guild Wars to run in windowed mode.\n"
    "Note: You can easily switch to windowed mode by pressing ALT + Enter or clicking the window icon in the upper right corner while the game is already running.\n"
    "-windowedfullscreen\n"
    "Forces Guild Wars to run in windowed fullscreen mode.";
  wxMessageBox( help, Language->CommandLineHelpTitle, wxOK);
}

void uMod_Frame::UpdateLaunchButtonState(void)
{
  wxString exe_path = ExePathText->GetValue();
  exe_path.Trim();
  exe_path.Trim(false);
  bool has_exe = !exe_path.IsEmpty();
  LaunchButton->Enable( has_exe);
}

void uMod_Frame::LoadLauncherSettings(void)
{
  wxArrayString games, cmd;
  if (GetInjectedGames( games, cmd) != 0)
  {
    LastError.Empty();
    return;
  }

  if (games.GetCount()>0)
  {
    ExePathText->SetValue( games[0]);
    CommandLineText->SetValue( cmd[0]);
    if (TextureRipperPage!=NULL) TextureRipperPage->SetExeName(games[0]);
  }
}

void uMod_Frame::SaveLauncherSettings(const wxString &exe_path, const wxString &command_line)
{
  wxArrayString games, cmd;
  if (!exe_path.IsEmpty())
  {
    games.Add( exe_path);
    cmd.Add( command_line);
  }
  if (SetInjectedGames( games, cmd))
  {
    wxMessageBox( LastError, "ERROR", wxOK|wxICON_ERROR);
    LastError.Empty();
  }
}



int uMod_Frame::ActivateGamesControl(void)
{
  MenuMain->Enable( ID_Menu_LoadTemplate, true);
  MenuMain->Enable( ID_Menu_SaveTemplate, true);
  MenuMain->Enable( ID_Menu_SaveTemplateAs, true);
  MenuMain->Enable( ID_Menu_SetDefaultTemplate, true);


  OpenButton->Enable( true);
  DirectoryButton->Enable( true);
  UpdateButton->Enable( true);
  ReloadButton->Enable( true);

  return 0;
}

int uMod_Frame::DeactivateGamesControl(void)
{
  MenuMain->Enable( ID_Menu_LoadTemplate, false);
  MenuMain->Enable( ID_Menu_SaveTemplate, false);
  MenuMain->Enable( ID_Menu_SaveTemplateAs, false);
  MenuMain->Enable( ID_Menu_SetDefaultTemplate, false);


  OpenButton->Enable( false);
  DirectoryButton->Enable( false);
  UpdateButton->Enable( false);
  ReloadButton->Enable( false);
  return 0;
}

#define DI_FILE "uMod_DI_Games.txt"
int uMod_Frame::GetInjectedGames( wxArrayString &games, wxArrayString &cmd)
{
  wxFile file;

  if (!file.Access( DI_FILE, wxFile::read)) {LastError << Language->Error_FileOpen << "\n" << DI_FILE; return -1;}
  file.Open( DI_FILE, wxFile::read);
  if (!file.IsOpened()) {LastError << Language->Error_FileOpen << "\n" << DI_FILE ; return -1;}

  unsigned len = file.Length();

  unsigned char* buffer;
  try {buffer = new unsigned char [len+2];}
  catch (...) {LastError << Language->Error_Memory; return -1;}

  unsigned int result = file.Read( buffer, len);
  file.Close();

  if (result != len) {delete [] buffer; LastError << Language->Error_FileRead<<"\n" << DI_FILE; return -1;}

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

  file.Open( DI_FILE, wxFile::write);
  if (!file.IsOpened()) {LastError << Language->Error_FileOpen << "\n" << DI_FILE ; return -1;}
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


#define SAVE_FILE "uMod_SaveFiles.txt"

int uMod_Frame::LoadTemplate(void)
{
  wxFile file;
  if (!file.Access(SAVE_FILE, wxFile::read)) {LastError << Language->Error_FileOpen << "\n" << SAVE_FILE; return -1;}
  file.Open(SAVE_FILE, wxFile::read);
  if (!file.IsOpened()) {LastError << Language->Error_FileOpen << "\n" << SAVE_FILE ; return -1;}

  unsigned len = file.Length();

  unsigned char* buffer;
  try {buffer = new unsigned char [len+2];}
  catch (...) {LastError << Language->Error_Memory; return -1;}

  unsigned int result = file.Read( buffer, len);
  file.Close();

  if (result != len) {delete [] buffer; LastError << Language->Error_FileRead<<"\n" << SAVE_FILE; return -1;}

  wchar_t *buff = (wchar_t*)buffer;
  len/=2;
  buff[len]=0;

  wxString content;
  content =  buff;
  delete [] buffer;

  wxStringTokenizer token( content, "\n");

  int num = token.CountTokens();

  SaveFile_Exe.Empty();
  SaveFile_Exe.Alloc(num+10);
  SaveFile_Name.Empty();
  SaveFile_Name.Alloc(num+10);

  wxString line;
  wxString exe;
  wxString name;
  for (int i=0; i<num; i++)
  {
    line = token.GetNextToken();
    exe = line.BeforeFirst('|');
    name = line.AfterFirst('|');
    name.Replace("\r","");
    SaveFile_Exe.Add( exe);
    SaveFile_Name.Add( name);
  }
  return 0;
}

int uMod_Frame::SaveTemplate(void)
{
  wxFile file;
  file.Open(SAVE_FILE, wxFile::write);
  if (!file.IsOpened()) {LastError << Language->Error_FileOpen << "\n" << SAVE_FILE ; return -1;}
  wxString content;

  int num = SaveFile_Exe.GetCount();
  for (int i=0; i<num; i++)
  {
    content = SaveFile_Exe[i];
    content << "|" << SaveFile_Name[i] << "\n";
    file.Write( content.wc_str(), content.Len()*2);
  }
  file.Close();
  return 0;
}
