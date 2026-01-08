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
#include <wx/artprov.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/tooltip.h>

static const wchar_t* DEFAULT_MODS_FILE = L"uMod_DefaultMods.txt";
static const wchar_t* DEFAULT_MODS_STATE_FILE = L"uMod_DefaultModsEnabled.txt";


uMod_GamePage::uMod_GamePage( wxWindow *parent, const wxString &exe, const wxString &save, PipeStruct &pipe, uMod_Frame *frame)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), Sender(pipe)
{
  (void)save;
  MainFrame = frame;
  ExeName = exe;

  CheckBoxHSizers = NULL;
  CheckButtonUp = NULL;
  CheckButtonDown = NULL;
  CheckButtonDelete = NULL;
  CheckBoxes = NULL;
  SavedTexturesList = NULL;
  SavedTexturesLabel = NULL;
  PackageNameLabel = NULL;
  PackageAuthorLabel = NULL;
  PackageName = NULL;
  PackageAuthor = NULL;
  SavePackageButton = NULL;

  MainSizer = new wxBoxSizer(wxVERTICAL);

  Notebook = new wxNotebook( this, wxID_ANY);
  Notebook->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  MainSizer->Add( (wxWindow*) Notebook, 1, wxEXPAND , 0 );

  LauncherPanel = new wxScrolledWindow( Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
  ModMakerPanel = new wxScrolledWindow( Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);

  LauncherSizer = new wxBoxSizer(wxVERTICAL);
  ModMakerSizer = new wxBoxSizer(wxVERTICAL);

  const wxSize launcher_button_size(200, 28);
  const wxSize launch_button_size(200, 56);
  wxBoxSizer *launchRow = new wxBoxSizer(wxHORIZONTAL);
  LaunchButton = new wxButton( LauncherPanel, wxID_ANY, Language->ButtonLaunch, wxDefaultPosition, launch_button_size);
  LaunchButton->SetBackgroundColour(wxColour(0, 120, 215));
  LaunchButton->SetForegroundColour(*wxWHITE);
  CommandLine = new wxTextCtrl( LauncherPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
  CommandLine->SetHint( Language->CommandLineHint);
  launchRow->Add( (wxWindow*) LaunchButton, 0, wxRIGHT, 10);
  launchRow->Add( (wxWindow*) CommandLine, 1, wxEXPAND, 0);
  LauncherSizer->Add( launchRow, 0, wxEXPAND, 0);
  LauncherSizer->AddSpacer(10);

  wxBoxSizer *exeRow = new wxBoxSizer(wxHORIZONTAL);
  LocateExeButton = new wxButton( LauncherPanel, wxID_ANY, Language->ButtonLocateExe, wxDefaultPosition, launcher_button_size);
  ExePath = new wxTextCtrl( LauncherPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  ExeStatus = new wxStaticText( LauncherPanel, wxID_ANY, "");
  ExeStatus->SetForegroundColour(wxColour(0, 160, 0));
  exeRow->Add( (wxWindow*) LocateExeButton, 0, wxRIGHT, 10);
  exeRow->Add( (wxWindow*) ExePath, 1, wxEXPAND, 0);
  exeRow->Add( (wxWindow*) ExeStatus, 0, wxLEFT, 8);
  LauncherSizer->Add( exeRow, 0, wxEXPAND, 0);
  LauncherSizer->AddSpacer(10);

  wxBoxSizer *openRow = new wxBoxSizer(wxHORIZONTAL);
  OpenButton = new wxButton( LauncherPanel, ID_Button_Open, Language->ButtonOpen, wxDefaultPosition, launcher_button_size);
  OpenButtonHint = new wxStaticText( LauncherPanel, wxID_ANY, Language->SelectModsHint);
  OpenButtonHint->SetForegroundColour(wxColour(120, 120, 120));
  openRow->Add( (wxWindow*) OpenButton, 0, wxRIGHT, 10);
  openRow->Add( (wxWindow*) OpenButtonHint, 0, wxALIGN_CENTER_VERTICAL, 0);
  LauncherSizer->Add( openRow, 0, wxALIGN_LEFT, 0);
  LoadDefaultMods = new wxCheckBox( LauncherPanel, wxID_ANY, Language->LoadDefaultMods);
  LauncherSizer->Add( (wxWindow*) LoadDefaultMods, 0, wxTOP, 6);
  LauncherSizer->AddSpacer(10);

  ModsSizer = new wxStaticBoxSizer(wxVERTICAL, LauncherPanel, Language->LoadedMods);
  LauncherSizer->Add( ModsSizer, 1, wxEXPAND, 0);

  LauncherPanel->SetSizer(LauncherSizer);
  LauncherPanel->SetScrollRate(0, 20);
  LauncherSizer->FitInside(LauncherPanel);

  SizerKeys[0] = new wxBoxSizer(wxHORIZONTAL);
  SizerKeys[1] = new wxBoxSizer(wxHORIZONTAL);

  TextKeyBack = new wxTextCtrl(ModMakerPanel, wxID_ANY, Language->KeyBack, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  SizerKeys[0]->Add( (wxWindow*) TextKeyBack, 1, wxEXPAND, 0);
  ChoiceKeyBack = new wxChoice( ModMakerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, Language->KeyStrings);
  SizerKeys[1]->Add( (wxWindow*) ChoiceKeyBack, 1, wxEXPAND, 0);

  TextKeySave = new wxTextCtrl(ModMakerPanel, wxID_ANY, Language->KeySave, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  SizerKeys[0]->Add( (wxWindow*) TextKeySave, 1, wxEXPAND, 0);
  ChoiceKeySave = new wxChoice( ModMakerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, Language->KeyStrings);
  SizerKeys[1]->Add( (wxWindow*) ChoiceKeySave, 1, wxEXPAND, 0);

  TextKeyNext = new wxTextCtrl(ModMakerPanel, wxID_ANY, Language->KeyNext, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  SizerKeys[0]->Add( (wxWindow*) TextKeyNext, 1, wxEXPAND, 0);
  ChoiceKeyNext = new wxChoice( ModMakerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, Language->KeyStrings);
  SizerKeys[1]->Add( (wxWindow*) ChoiceKeyNext, 1, wxEXPAND, 0);

  ModMakerSizer->Add( SizerKeys[0], 0, wxEXPAND, 0);
  ModMakerSizer->Add( SizerKeys[1], 0, wxEXPAND, 0);


  FontColourSizer = new wxBoxSizer(wxHORIZONTAL);
  FontColour[0] = new wxTextCtrl(ModMakerPanel, wxID_ANY, Language->FontColour, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  FontColour[1] = new wxTextCtrl(ModMakerPanel, wxID_ANY, "255", wxDefaultPosition, wxDefaultSize);
  FontColour[2] = new wxTextCtrl(ModMakerPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize);
  FontColour[3] = new wxTextCtrl(ModMakerPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize);
  for (int i=0; i<4; i++) FontColourSizer->Add( (wxWindow*) FontColour[i], 1, wxEXPAND, 0);

  TextureColourSizer = new wxBoxSizer(wxHORIZONTAL);
  TextureColour[0] = new wxTextCtrl(ModMakerPanel, wxID_ANY, Language->TextureColour, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  TextureColour[1] = new wxTextCtrl(ModMakerPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize);
  TextureColour[2] = new wxTextCtrl(ModMakerPanel, wxID_ANY, "255", wxDefaultPosition, wxDefaultSize);
  TextureColour[3] = new wxTextCtrl(ModMakerPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize);
  for (int i=0; i<4; i++) TextureColourSizer->Add( (wxWindow*) TextureColour[i], 1, wxEXPAND, 0);


  ModMakerSizer->Add( FontColourSizer, 0, wxEXPAND, 0);
  ModMakerSizer->Add( TextureColourSizer, 0, wxEXPAND, 0);

  SaveSingleTexture = new wxCheckBox( ModMakerPanel, -1, Language->CheckBoxSaveSingleTexture);
  ModMakerSizer->Add( (wxWindow*) SaveSingleTexture, 0, wxEXPAND, 0);

  SaveAllTextures = new wxCheckBox( ModMakerPanel, -1, Language->CheckBoxSaveAllTextures);
  ModMakerSizer->Add( (wxWindow*) SaveAllTextures, 0, wxEXPAND, 0);

  wxBoxSizer *savePathRow = new wxBoxSizer(wxHORIZONTAL);
  DirectoryButton = new wxButton( ModMakerPanel, ID_Button_Path, Language->ButtonDirectory, wxDefaultPosition, wxSize(180,24));
  SavePath = new wxTextCtrl(ModMakerPanel, wxID_ANY, Language->TextCtrlSavePath, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  savePathRow->Add( (wxWindow*) DirectoryButton, 0, wxRIGHT, 10);
  savePathRow->Add( (wxWindow*) SavePath, 1, wxEXPAND, 0);
  ModMakerSizer->Add( savePathRow, 0, wxEXPAND, 0);

  ModMakerSizer->AddSpacer(10);

  wxBoxSizer *updateRow = new wxBoxSizer(wxHORIZONTAL);
  UpdateButton = new wxButton( ModMakerPanel, ID_Button_Update, Language->ButtonUpdate, wxDefaultPosition, wxSize(140,24));
  ReloadButton = new wxButton( ModMakerPanel, ID_Button_Reload, Language->ButtonReload, wxDefaultPosition, wxSize(140,24));
  updateRow->Add( (wxWindow*) UpdateButton, 0, wxRIGHT, 10);
  updateRow->Add( (wxWindow*) ReloadButton, 0, wxEXPAND, 0);
  ModMakerSizer->Add( updateRow, 0, wxEXPAND, 0);

  ModMakerSizer->AddSpacer(10);

  SavedTexturesLabel = new wxStaticText(ModMakerPanel, wxID_ANY, Language->SavedTexturesLabel);
  ModMakerSizer->Add( (wxWindow*) SavedTexturesLabel, 0, wxEXPAND, 0);

  SavedTexturesList = new wxListCtrl(ModMakerPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 240),
                                     wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
  SavedTexturesList->InsertColumn(0, Language->SavedTexturesColumn, wxLIST_FORMAT_LEFT, 400);
#if wxCHECK_VERSION(3,1,0)
  SavedTexturesList->EnableCheckBoxes(true);
#endif
  wxImageList *saved_images = new wxImageList(24, 24);
  saved_images->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(24, 24)));
  SavedTexturesList->AssignImageList(saved_images, wxIMAGE_LIST_SMALL);
  ModMakerSizer->Add( (wxWindow*) SavedTexturesList, 1, wxEXPAND, 0);

  wxBoxSizer *packageNameRow = new wxBoxSizer(wxHORIZONTAL);
  PackageNameLabel = new wxStaticText(ModMakerPanel, wxID_ANY, Language->PackageNameLabel);
  packageNameRow->Add( (wxWindow*) PackageNameLabel, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);
  PackageName = new wxTextCtrl(ModMakerPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
  packageNameRow->Add( (wxWindow*) PackageName, 1, wxEXPAND, 0);
  ModMakerSizer->Add( packageNameRow, 0, wxEXPAND, 0);

  wxBoxSizer *packageAuthorRow = new wxBoxSizer(wxHORIZONTAL);
  PackageAuthorLabel = new wxStaticText(ModMakerPanel, wxID_ANY, Language->PackageAuthorLabel);
  packageAuthorRow->Add( (wxWindow*) PackageAuthorLabel, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);
  PackageAuthor = new wxTextCtrl(ModMakerPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
  packageAuthorRow->Add( (wxWindow*) PackageAuthor, 1, wxEXPAND, 0);
  ModMakerSizer->Add( packageAuthorRow, 0, wxEXPAND, 0);

  SavePackageButton = new wxButton(ModMakerPanel, ID_Button_SavePackage, Language->ButtonSavePackage, wxDefaultPosition, wxSize(180, 28));
  ModMakerSizer->Add( (wxWindow*) SavePackageButton, 0, wxTOP, 6);

  NumberOfEntry = 0;
  MaxNumberOfEntry = 1024;
  if (GetMemory( CheckBoxes, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  if (GetMemory( CheckBoxHSizers, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return ;}
  if (GetMemory( CheckButtonUp, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  if (GetMemory( CheckButtonDown, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  if (GetMemory( CheckButtonDelete, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  SavePath->SetValue(Language->TextCtrlSavePath);

  ModMakerPanel->SetSizer(ModMakerSizer);
  ModMakerPanel->SetScrollRate(0, 20);
  ModMakerSizer->FitInside(ModMakerPanel);

  Notebook->AddPage( LauncherPanel, Language->TabLauncher, true);
  Notebook->AddPage( ModMakerPanel, Language->TabModMaker, false);

  SetSizer(MainSizer);

  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonLaunch, this, LaunchButton->GetId());
  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonLocateExe, this, LocateExeButton->GetId());
  Bind( wxEVT_COMMAND_CHECKBOX_CLICKED, &uMod_GamePage::OnToggleLoadDefaultMods, this, LoadDefaultMods->GetId());

  UpdateLaunchState();
  LoadDefaultModsList();
  if (LoadDefaultMods->GetValue()) ApplyDefaultMods();
  else ClearModsList(false);

  RefreshSavedTextures();
}

uMod_GamePage::~uMod_GamePage(void)
{
  for (int i=0; i<NumberOfEntry; i++)
  {
    Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonUp, this, ID_Button_Texture+3*i);
    Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDown, this, ID_Button_Texture+3*i+1);
    Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDelete, this, ID_Button_Texture+3*i+2);
  }

  delete [] CheckBoxHSizers;
  delete [] CheckButtonUp;
  delete [] CheckButtonDown;
  delete [] CheckButtonDelete;
  delete [] CheckBoxes;
}

void uMod_GamePage::EnableOpenButton( bool enable)
{
  if (OpenButton!=NULL) OpenButton->Enable(enable);
  if (OpenButtonHint!=NULL) OpenButtonHint->Hide();
  if (LauncherPanel!=NULL)
  {
    LauncherPanel->Layout();
    LauncherSizer->FitInside(LauncherPanel);
  }
}

void uMod_GamePage::EnableGameControls( bool enable)
{
  if (DirectoryButton!=NULL) DirectoryButton->Enable(enable);
  if (UpdateButton!=NULL) UpdateButton->Enable(enable);
  if (ReloadButton!=NULL) ReloadButton->Enable(enable);
  if (SavePackageButton!=NULL) SavePackageButton->Enable(enable);
}

void uMod_GamePage::SetGameInfo( const wxString &exe, const wxString &save)
{
  (void)save;
  ExeName = exe;
  if (LoadDefaultMods->GetValue())
  {
    ApplyDefaultMods();
    UpdateGame();
  }
}

int uMod_GamePage::LoadLauncherSettings(void)
{
  if (MainFrame==NULL) return -1;
  wxArrayString games, cmd;
  if (MainFrame->GetInjectedGames(games, cmd)) return -1;
  if (games.GetCount()>0)
  {
    SetExePath( games[0]);
    if (cmd.GetCount()>0) CommandLine->SetValue( cmd[0]);
  }
  UpdateLaunchState();
  return 0;
}

void uMod_GamePage::OnButtonLaunch(wxCommandEvent& WXUNUSED(event))
{
  if (MainFrame==NULL) return;
  wxString exe_path = ExePath->GetValue();
  if (exe_path.IsEmpty()) return;
  wxString command_line = CommandLine->GetValue();
  PersistLauncherSettings( exe_path, command_line);
  MainFrame->LaunchGame( exe_path, command_line);
}

void uMod_GamePage::OnButtonLocateExe(wxCommandEvent& WXUNUSED(event))
{
  if (MainFrame==NULL) return;
  wxString file_name = wxFileSelector( Language->ChooseGame, "", "", "exe",  "binary (*.exe)|*.exe", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  if ( !file_name.empty() )
  {
    SetExePath( file_name);
    PersistLauncherSettings( file_name, CommandLine->GetValue());
  }
}

void uMod_GamePage::SetExePath(const wxString &path)
{
  ExePath->SetValue( path);
  UpdateLaunchState();
}

void uMod_GamePage::OnModCheck(wxCommandEvent& WXUNUSED(event))
{
  SaveDefaultModsList();
}

void uMod_GamePage::OnToggleLoadDefaultMods(wxCommandEvent& WXUNUSED(event))
{
  SaveDefaultModsState();
}

int uMod_GamePage::LoadDefaultModsList(void)
{
  DefaultMods.Empty();
  wxFile file;
  if (!file.Access( DEFAULT_MODS_FILE, wxFile::read))
  {
    if (LoadDefaultMods!=NULL) LoadDefaultMods->SetValue(false);
    return 0;
  }
  file.Open( DEFAULT_MODS_FILE, wxFile::read);
  if (!file.IsOpened()) return 0;

  unsigned len = file.Length();
  unsigned char* buffer;
  try {buffer = new unsigned char [len+2];}
  catch (...) {return -1;}

  unsigned int result = file.Read( buffer, len);
  file.Close();
  if (result != len) {delete [] buffer; return -1;}

  wchar_t *buff = (wchar_t*)buffer;
  len/=2;
  buff[len]=0;

  wxString content;
  content =  buff;
  delete [] buffer;

  wxStringTokenizer token( content, "\n");
  while (token.HasMoreTokens())
  {
    wxString entry = token.GetNextToken();
    entry.Replace("\r","");
    if (!entry.IsEmpty()) DefaultMods.Add(entry);
  }

  LoadDefaultModsState();
  return 0;
}

int uMod_GamePage::SaveDefaultModsList(void)
{
  DefaultMods.Empty();
  for (int i=0; i<NumberOfEntry; i++)
  {
    if (CheckBoxes[i]->GetValue()) DefaultMods.Add(Files[i]);
  }

  wxFile file;
  file.Open( DEFAULT_MODS_FILE, wxFile::write);
  if (!file.IsOpened()) return -1;
  wxString content;
  for (unsigned int i=0; i<DefaultMods.GetCount(); i++)
  {
    content = DefaultMods[i];
    content << "\n";
    file.Write( content.wc_str(), content.Len()*2);
  }
  file.Close();
  return 0;
}

int uMod_GamePage::LoadDefaultModsState(void)
{
  bool enabled = true;
  wxFile file;
  if (file.Access( DEFAULT_MODS_STATE_FILE, wxFile::read))
  {
    file.Open( DEFAULT_MODS_STATE_FILE, wxFile::read);
    if (file.IsOpened())
    {
      unsigned len = file.Length();
      unsigned char* buffer;
      try {buffer = new unsigned char [len+2];}
      catch (...) {return -1;}

      unsigned int result = file.Read( buffer, len);
      file.Close();
      if (result != len) {delete [] buffer; return -1;}

      wchar_t *buff = (wchar_t*)buffer;
      len/=2;
      buff[len]=0;
      enabled = (buff[0] == L'1');
      delete [] buffer;
    }
  }
  else
  {
    enabled = (DefaultMods.GetCount()>0);
  }

  if (LoadDefaultMods!=NULL) LoadDefaultMods->SetValue(enabled);
  return 0;
}

int uMod_GamePage::SaveDefaultModsState(void)
{
  if (LoadDefaultMods==NULL) return 0;
  wxFile file;
  file.Open( DEFAULT_MODS_STATE_FILE, wxFile::write);
  if (!file.IsOpened()) return -1;
  wxString content = LoadDefaultMods->GetValue() ? "1" : "0";
  file.Write( content.wc_str(), content.Len()*2);
  file.Close();
  return 0;
}

int uMod_GamePage::ApplyDefaultMods(void)
{
  if (DefaultMods.GetCount()==0) return 0;
  int added = 0;
  for (unsigned int i=0; i<DefaultMods.GetCount(); i++)
  {
    const wxString &file_name = DefaultMods[i];
    bool exists = false;
    for (int j=0; j<NumberOfEntry; j++) if (Files[j]==file_name) {exists = true; break;}
    if (!exists)
    {
      if (AddTextureInternal(file_name, false)==0) added++;
      else LastError.Empty();
    }
  }
  return added;
}

void uMod_GamePage::ResetConnection(void)
{
  Sender.Reset();
  GameOld.Init();
}

void uMod_GamePage::ClearModsList(bool clear_defaults)
{
  for (int i=0; i<NumberOfEntry; i++)
  {
    Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonUp, this, ID_Button_Texture+3*i);
    Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDown, this, ID_Button_Texture+3*i+1);
    Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDelete, this, ID_Button_Texture+3*i+2);

    CheckBoxHSizers[i]->Detach( (wxWindow*) CheckBoxes[i]);
    CheckBoxHSizers[i]->Detach( (wxWindow*) CheckButtonUp[i]);
    CheckBoxHSizers[i]->Detach( (wxWindow*) CheckButtonDown[i]);
    CheckBoxHSizers[i]->Detach( (wxWindow*) CheckButtonDelete[i]);

    ModsSizer->Detach( CheckBoxHSizers[i]);

    delete CheckBoxes[i];
    delete CheckButtonUp[i];
    delete CheckButtonDown[i];
    delete CheckButtonDelete[i];
    delete CheckBoxHSizers[i];
  }
  NumberOfEntry = 0;
  Files.Clear();
  if (clear_defaults) DefaultMods.Clear();

  LauncherPanel->Layout();
  LauncherSizer->FitInside(LauncherPanel);
}

void uMod_GamePage::UpdateLaunchState(void)
{
  wxString path = ExePath->GetValue();
  bool valid = false;
  if (!path.IsEmpty()) valid = wxFileName::FileExists(path);
  LaunchButton->Enable( valid);
  if (valid) ExeStatus->SetLabel( L"\u2713");
  else ExeStatus->SetLabel( "");
}

int uMod_GamePage::PersistLauncherSettings(const wxString &exe_path, const wxString &command_line)
{
  if (MainFrame==NULL || exe_path.IsEmpty()) return -1;
  wxArrayString games, cmd;
  if (MainFrame->GetInjectedGames(games, cmd))
  {
    games.Clear();
    cmd.Clear();
  }
  int index = -1;
  int num = games.GetCount();
  for (int i=0; i<num; i++) if (games[i]==exe_path) {index = i; break;}
  if (index<0)
  {
    games.Add( exe_path);
    cmd.Add( command_line);
  }
  else
  {
    int cmd_count = cmd.GetCount();
    if (index>=cmd_count) cmd.Add( command_line);
    else cmd[index] = command_line;
  }
  return MainFrame->SetInjectedGames(games, cmd);
}

int uMod_GamePage::SetSavePath( const wxString &path)
{
  wxString save_path = Language->TextCtrlSavePath;
  save_path << path;
  SavePath->SetValue(save_path);
  Game.SetSavePath( path);
  RefreshSavedTextures();
  return 0;
}

void uMod_GamePage::RefreshSavedTextures(void)
{
  if (SavedTexturesList==NULL) return;
  SavedTexturesList->DeleteAllItems();
  SavedTexturePaths.Clear();

  wxString save_path = Game.GetSavePath();
  if (save_path.IsEmpty() || !wxDirExists(save_path))
  {
    long index = SavedTexturesList->InsertItem(0, Language->SavedTexturesHintNoPath, 0);
    SavedTexturesList->SetItemTextColour(index, wxColour(120, 120, 120));
    return;
  }

  wxDir dir(save_path);
  if (!dir.IsOpened())
  {
    long index = SavedTexturesList->InsertItem(0, Language->SavedTexturesHintNoPath, 0);
    SavedTexturesList->SetItemTextColour(index, wxColour(120, 120, 120));
    return;
  }

  wxString filename;
  bool found = dir.GetFirst(&filename, "*.dds", wxDIR_FILES);
  if (!found)
  {
    long index = SavedTexturesList->InsertItem(0, Language->SavedTexturesHintEmpty, 0);
    SavedTexturesList->SetItemTextColour(index, wxColour(120, 120, 120));
    return;
  }

  long item = 0;
  while (found)
  {
    wxFileName file_name(save_path, filename);
    SavedTexturePaths.Add(file_name.GetFullPath());
    long index = SavedTexturesList->InsertItem(item, filename, 0);
    SavedTexturesList->SetItemData(index, SavedTexturePaths.GetCount() - 1);
#if wxCHECK_VERSION(3,1,0)
    SavedTexturesList->CheckItem(index, true);
#endif
    item++;
    found = dir.GetNext(&filename);
  }
  SavedTexturesList->SetColumnWidth(0, wxLIST_AUTOSIZE);
}


int uMod_GamePage::AddTexture( const wxString &file_name)
{
  return AddTextureInternal(file_name, true);
}

int uMod_GamePage::AddTextureInternal( const wxString &file_name, bool update_game)
{
  if (NumberOfEntry>=MaxNumberOfEntry)
  {
    if (GetMoreMemory( CheckBoxes, MaxNumberOfEntry, MaxNumberOfEntry+1024)) {LastError = Language->Error_Memory; return -1;}
    if (GetMoreMemory( CheckBoxHSizers, MaxNumberOfEntry, MaxNumberOfEntry+1024)) {LastError = Language->Error_Memory; return -1;}
    if (GetMoreMemory( CheckButtonUp, MaxNumberOfEntry, MaxNumberOfEntry+1024)) {LastError = Language->Error_Memory; return -1;}
    if (GetMoreMemory( CheckButtonDown, MaxNumberOfEntry, MaxNumberOfEntry+1024)) {LastError = Language->Error_Memory; return -1;}
    if (GetMoreMemory( CheckButtonDelete, MaxNumberOfEntry, MaxNumberOfEntry+1024)) {LastError = Language->Error_Memory; return -1;}
    MaxNumberOfEntry+=1024;
  }
  uMod_File file( file_name);
  if (!file.FileSupported()) {LastError << Language->Error_FileNotSupported << "\n" << file_name; return -1;}

  wxString tool_tip;
  file.GetComment( tool_tip);

  CheckBoxHSizers[NumberOfEntry] = new wxBoxSizer(wxHORIZONTAL);
  CheckBoxes[NumberOfEntry] = new wxCheckBox( LauncherPanel, -1, file_name);
  CheckBoxes[NumberOfEntry]->SetValue( true);
  CheckBoxes[NumberOfEntry]->SetToolTip( tool_tip);
  CheckBoxes[NumberOfEntry]->Bind( wxEVT_COMMAND_CHECKBOX_CLICKED, &uMod_GamePage::OnModCheck, this);

  wchar_t button_txt[2];
  button_txt[0] = 8657;
  button_txt[1] = 0;
  CheckButtonUp[NumberOfEntry] = new wxButton( LauncherPanel, ID_Button_Texture+3*NumberOfEntry, button_txt, wxDefaultPosition, wxSize(24,24));
  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonUp, this, ID_Button_Texture+3*NumberOfEntry);

  button_txt[0] = 8659;
  CheckButtonDown[NumberOfEntry] = new wxButton( LauncherPanel, ID_Button_Texture+3*NumberOfEntry+1, button_txt, wxDefaultPosition, wxSize(24,24));
  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDown, this, ID_Button_Texture+3*NumberOfEntry+1);

  CheckButtonDelete[NumberOfEntry] = new wxButton( LauncherPanel, ID_Button_Texture+3*NumberOfEntry+2, L"X", wxDefaultPosition, wxSize(24,24));
  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDelete, this, ID_Button_Texture+3*NumberOfEntry+2);

  CheckBoxHSizers[NumberOfEntry]->Add( (wxWindow*) CheckBoxes[NumberOfEntry], 1, wxEXPAND, 0);
  CheckBoxHSizers[NumberOfEntry]->Add( (wxWindow*) CheckButtonUp[NumberOfEntry], 0, wxEXPAND, 0);
  CheckBoxHSizers[NumberOfEntry]->Add( (wxWindow*) CheckButtonDown[NumberOfEntry], 0, wxEXPAND, 0);
  CheckBoxHSizers[NumberOfEntry]->Add( (wxWindow*) CheckButtonDelete[NumberOfEntry], 0, wxEXPAND, 0);

  ModsSizer->Add( CheckBoxHSizers[NumberOfEntry], 0, wxEXPAND, 0);
  Files.Add( file_name);
  NumberOfEntry++;
  LauncherPanel->Layout();
  LauncherSizer->FitInside(LauncherPanel);

  SaveDefaultModsList();
  if (update_game) return UpdateGame();
  return 0;
}

int uMod_GamePage::GetSettings(void)
{
  int key_back = ChoiceKeyBack->GetSelection();
  int key_save = ChoiceKeySave->GetSelection();
  int key_next = ChoiceKeyNext->GetSelection();

  if (key_back==key_save && key_back!=wxNOT_FOUND) {LastError << Language->Error_KeyTwice; return 1;}
  if (key_back==key_next && key_back!=wxNOT_FOUND) {LastError << Language->Error_KeyTwice; return 1;}
  if (key_save==key_next && key_save!=wxNOT_FOUND) {LastError << Language->Error_KeyTwice; return 1;}

  bool save_single = SaveSingleTexture->GetValue();
  bool save_all = SaveAllTextures->GetValue();
  wxString path = Game.GetSavePath();
  if ( (save_single || save_all) && path.Len()==0) {LastError << Language->Error_NoSavePath; return 1;}

  if ( save_single && ( key_back==wxNOT_FOUND || key_save==wxNOT_FOUND || key_next==wxNOT_FOUND) ) {LastError << Language->Error_KeyNotSet; return 1;}

  if (key_back!=wxNOT_FOUND) Game.SetKeyBack(key_back);
  if (key_save!=wxNOT_FOUND) Game.SetKeySave(key_save);
  if (key_next!=wxNOT_FOUND) Game.SetKeyNext(key_next);

  Game.SetSaveSingleTexture( save_single);
  Game.SetSaveAllTextures( save_all);

  int colour[3];
  colour[0] = GetColour( FontColour[1], 255);
  colour[1] = GetColour( FontColour[2], 0);
  colour[2] = GetColour( FontColour[3], 0);
  SetColour( &FontColour[1], colour);
  Game.SetFontColour(colour);

  colour[0] = GetColour( TextureColour[1], 0);
  colour[1] = GetColour( TextureColour[2], 255);
  colour[2] = GetColour( TextureColour[3], 0);
  SetColour( &TextureColour[1], colour);
  Game.SetTextureColour(colour);

  Game.SetFiles( Files);

  bool *checked = NULL;
  if (GetMemory( checked, NumberOfEntry)) {LastError = Language->Error_Memory; return -1;}
  for (int i=0; i<NumberOfEntry; i++) checked[i] = CheckBoxes[i]->GetValue();
  Game.SetChecked( checked, NumberOfEntry);
  delete [] checked;

  return 0;
}

int uMod_GamePage::UpdateGame(void)
{
  if (MainFrame!=NULL && !MainFrame->IsGameActive())
  {
    if (GetSettings()) LastError.Empty();
    return 0;
  }
  if (int ret = GetSettings()) return ret;

  if (int ret = Sender.Send( Game, GameOld, false))
  {
    LastError = Language->Error_Send;
    LastError << "\n" << Sender.LastError;
    Sender.LastError.Empty();
    return ret;
  }

  GameOld = Game;
  RefreshSavedTextures();
  return 0;
}


int uMod_GamePage::ReloadGame(void)
{
  if (MainFrame!=NULL && !MainFrame->IsGameActive())
  {
    if (GetSettings()) LastError.Empty();
    return 0;
  }
  if (int ret = GetSettings()) return ret;

  if (int ret = Sender.Send( Game, GameOld, true))
  {
    LastError = Language->Error_Send;
    LastError << "\n" << Sender.LastError;
    Sender.LastError.Empty();
    return ret;
  }

  GameOld = Game;
  RefreshSavedTextures();
  return 0;
}

int uMod_GamePage::SavePackage(void)
{
  LastError.Empty();
  wxString save_path = Game.GetSavePath();
  if (save_path.IsEmpty())
  {
    LastError = Language->Error_NoSavePath;
    return -1;
  }

  wxString package_name = PackageName->GetValue();
  package_name.Trim(true);
  package_name.Trim(false);
  if (package_name.IsEmpty())
  {
    LastError = Language->Error_PackageNameRequired;
    return -1;
  }

  wxArrayString selected_files;
  long item = -1;
  while (true)
  {
    item = SavedTexturesList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
    if (item == -1) break;
#if wxCHECK_VERSION(3,1,0)
    if (!SavedTexturesList->IsItemChecked(item)) continue;
#endif
    long data = SavedTexturesList->GetItemData(item);
    if (data >= 0 && data < (long)SavedTexturePaths.GetCount())
    {
      selected_files.Add(SavedTexturePaths[(size_t)data]);
    }
  }

  if (selected_files.IsEmpty())
  {
    LastError = Language->Error_NoTexturesSelected;
    return -1;
  }

  wxFileName output_name(save_path, package_name);
  if (output_name.GetExt().IsEmpty()) output_name.SetExt("tpf");
  wxString output_path = output_name.GetFullPath();

  wxString error;
  wxString author = PackageAuthor->GetValue();
  author.Trim(true);
  author.Trim(false);
  if (uMod_File::CreateTpfPackage(output_path, selected_files, author, error))
  {
    LastError = error;
    return -1;
  }

  LastError = Language->PackageSaved;
  LastError << "\n" << output_path;
  return 0;
}


int uMod_GamePage::SetColour( wxTextCtrl** txt, int *colour)
{
  wxString temp;
  for (int i=0; i<3; i++)
  {
    temp.Empty();
    temp << colour[i];
    txt[i]->SetValue( temp);
  }
  return 0;
}

int uMod_GamePage::GetColour( wxTextCtrl* txt, int def)
{
  wxString temp = txt->GetValue();
  long colour;
  if (temp.ToLong(&colour))
  {
    if (colour<0) colour=0;
    else if (colour>255) colour=255;
  }
  else colour = def;
  return colour;
}


void uMod_GamePage::OnButtonUp(wxCommandEvent& event)
{
  int id = (event.GetId() - ID_Button_Texture)/3;
  if (id <=0 || id>= NumberOfEntry) return;

  wxString cpy_str = Files[id];
  Files[id] = Files[id-1];
  Files[id-1] = cpy_str;

  CheckBoxes[id]->SetLabel(Files[id]);
  CheckBoxes[id-1]->SetLabel(Files[id-1]);

  bool cpy_checked = CheckBoxes[id]->GetValue();
  CheckBoxes[id]->SetValue(CheckBoxes[id-1]->GetValue());
  CheckBoxes[id-1]->SetValue(cpy_checked);

  cpy_str = CheckBoxes[id]->GetToolTip()->GetTip();
  wxString cpy_str2 = CheckBoxes[id-1]->GetToolTip()->GetTip();
  CheckBoxes[id]->SetToolTip(cpy_str2);
  CheckBoxes[id-1]->SetToolTip(cpy_str);

}

void uMod_GamePage::OnButtonDown(wxCommandEvent& event)
{
  int id = (event.GetId() - ID_Button_Texture-1)/3;
  if (id <0 || id>= NumberOfEntry-1) return;

  wxString cpy_str = Files[id];
  Files[id] = Files[id+1];
  Files[id+1] = cpy_str;

  CheckBoxes[id]->SetLabel(Files[id]);
  CheckBoxes[id+1]->SetLabel(Files[id+1]);

  bool cpy_checked = CheckBoxes[id]->GetValue();
  CheckBoxes[id]->SetValue(CheckBoxes[id+1]->GetValue());
  CheckBoxes[id+1]->SetValue(cpy_checked);

  cpy_str = CheckBoxes[id]->GetToolTip()->GetTip();
  wxString cpy_str2 = CheckBoxes[id+1]->GetToolTip()->GetTip();
  CheckBoxes[id]->SetToolTip(cpy_str2);
  CheckBoxes[id+1]->SetToolTip(cpy_str);
}

void uMod_GamePage::OnButtonDelete(wxCommandEvent& event)
{
  int id = (event.GetId() - ID_Button_Texture-2)/3;
  if (id <0 || id>= NumberOfEntry) return;

  for (int i=id+1; i<NumberOfEntry; i++) CheckBoxes[i-1]->SetLabel(Files[i]);
  for (int i=id+1; i<NumberOfEntry; i++) CheckBoxes[i-1]->SetValue(CheckBoxes[i]->GetValue());
  wxString cpy_str;
  for (int i=id+1; i<NumberOfEntry; i++)
  {
    cpy_str = CheckBoxes[i]->GetToolTip()->GetTip();
    CheckBoxes[i-1]->SetToolTip(cpy_str);
  }

  Files.RemoveAt(id, 1);
  NumberOfEntry--;


  Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonUp, this, ID_Button_Texture+3*NumberOfEntry);
  Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDown, this, ID_Button_Texture+3*NumberOfEntry+1);
  Unbind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonDelete, this, ID_Button_Texture+3*NumberOfEntry+2);


  CheckBoxHSizers[NumberOfEntry]->Detach( (wxWindow*) CheckBoxes[NumberOfEntry]);
  CheckBoxHSizers[NumberOfEntry]->Detach( (wxWindow*) CheckButtonUp[NumberOfEntry]);
  CheckBoxHSizers[NumberOfEntry]->Detach( (wxWindow*) CheckButtonDown[NumberOfEntry]);
  CheckBoxHSizers[NumberOfEntry]->Detach( (wxWindow*) CheckButtonDelete[NumberOfEntry]);

  ModsSizer->Detach( CheckBoxHSizers[NumberOfEntry]);

  delete CheckBoxes[NumberOfEntry];
  delete CheckButtonUp[NumberOfEntry];
  delete CheckButtonDown[NumberOfEntry];
  delete CheckButtonDelete[NumberOfEntry];
  delete CheckBoxHSizers[NumberOfEntry];

  SaveDefaultModsList();

  LauncherPanel->Layout();
  LauncherSizer->FitInside(LauncherPanel);
}


int uMod_GamePage::UpdateLanguage(void)
{
  Notebook->SetPageText( 0, Language->TabLauncher);
  Notebook->SetPageText( 1, Language->TabModMaker);
  LaunchButton->SetLabel( Language->ButtonLaunch);
  LocateExeButton->SetLabel( Language->ButtonLocateExe);
  CommandLine->SetHint( Language->CommandLineHint);
  OpenButton->SetLabel( Language->ButtonOpen);
  OpenButtonHint->SetLabel( Language->SelectModsHint);
  DirectoryButton->SetLabel( Language->ButtonDirectory);
  UpdateButton->SetLabel( Language->ButtonUpdate);
  ReloadButton->SetLabel( Language->ButtonReload);
  if (SavedTexturesLabel!=NULL) SavedTexturesLabel->SetLabel( Language->SavedTexturesLabel);
  if (SavedTexturesList!=NULL)
  {
    wxListItem column;
    column.SetText(Language->SavedTexturesColumn);
    SavedTexturesList->SetColumn(0, column);
    SavedTexturesList->SetColumnWidth(0, wxLIST_AUTOSIZE);
  }
  if (PackageNameLabel!=NULL) PackageNameLabel->SetLabel(Language->PackageNameLabel);
  if (PackageAuthorLabel!=NULL) PackageAuthorLabel->SetLabel(Language->PackageAuthorLabel);
  if (SavePackageButton!=NULL) SavePackageButton->SetLabel( Language->ButtonSavePackage);
  if (ModsSizer!=NULL && ModsSizer->GetStaticBox()!=NULL)
  {
    ModsSizer->GetStaticBox()->SetLabel( Language->LoadedMods);
  }

  TextKeyBack->SetValue( Language->KeyBack);
  TextKeySave->SetValue( Language->KeySave);
  TextKeyNext->SetValue( Language->KeyNext);
  FontColour[0]->SetValue( Language->FontColour);
  TextureColour[0]->SetValue( Language->TextureColour);
  SaveAllTextures->SetLabel( Language->CheckBoxSaveAllTextures);
  SaveSingleTexture->SetLabel( Language->CheckBoxSaveSingleTexture);
  LoadDefaultMods->SetLabel( Language->LoadDefaultMods);
  wxString temp = Language->TextCtrlSavePath;
  temp << Game.GetSavePath();
  SavePath->SetValue( temp);
  RefreshSavedTextures();
  return 0;
}
