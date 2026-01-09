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
#include "zip.h"
#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/renderer.h>
#include <wx/tooltip.h>
#include <cstring>

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
  SuppressDefaultModsSave = false;

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

  wxBoxSizer *savePathRow = new wxBoxSizer(wxHORIZONTAL);
  DirectoryButton = new wxButton( ModMakerPanel, ID_Button_Path, Language->ButtonDirectory, wxDefaultPosition, wxSize(180,24));
  SavePath = new wxTextCtrl(ModMakerPanel, wxID_ANY, Language->TextCtrlSavePath, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  savePathRow->Add( (wxWindow*) DirectoryButton, 0, wxRIGHT, 10);
  savePathRow->Add( (wxWindow*) SavePath, 1, wxEXPAND, 0);
  ModMakerSizer->Add( savePathRow, 0, wxEXPAND, 0);
  ModMakerSizer->AddSpacer(10);

  const wxSize key_button_size(180, 24);
  wxBoxSizer *keyBackRow = new wxBoxSizer(wxHORIZONTAL);
  KeyBackButton = new wxButton(ModMakerPanel, wxID_ANY, Language->KeyBack, wxDefaultPosition, key_button_size);
  ChoiceKeyBack = new wxChoice( ModMakerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, Language->KeyStrings);
  keyBackRow->Add( (wxWindow*) KeyBackButton, 0, wxRIGHT, 10);
  keyBackRow->Add( (wxWindow*) ChoiceKeyBack, 1, wxEXPAND, 0);
  ModMakerSizer->Add( keyBackRow, 0, wxEXPAND | wxBOTTOM, 6);

  wxBoxSizer *keySaveRow = new wxBoxSizer(wxHORIZONTAL);
  KeySaveButton = new wxButton(ModMakerPanel, wxID_ANY, Language->KeySave, wxDefaultPosition, key_button_size);
  ChoiceKeySave = new wxChoice( ModMakerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, Language->KeyStrings);
  keySaveRow->Add( (wxWindow*) KeySaveButton, 0, wxRIGHT, 10);
  keySaveRow->Add( (wxWindow*) ChoiceKeySave, 1, wxEXPAND, 0);
  ModMakerSizer->Add( keySaveRow, 0, wxEXPAND | wxBOTTOM, 6);

  wxBoxSizer *keyNextRow = new wxBoxSizer(wxHORIZONTAL);
  KeyNextButton = new wxButton(ModMakerPanel, wxID_ANY, Language->KeyNext, wxDefaultPosition, key_button_size);
  ChoiceKeyNext = new wxChoice( ModMakerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, Language->KeyStrings);
  keyNextRow->Add( (wxWindow*) KeyNextButton, 0, wxRIGHT, 10);
  keyNextRow->Add( (wxWindow*) ChoiceKeyNext, 1, wxEXPAND, 0);
  ModMakerSizer->Add( keyNextRow, 0, wxEXPAND, 0);


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

  wxBoxSizer *updateRow = new wxBoxSizer(wxHORIZONTAL);
  UpdateButton = new wxButton( ModMakerPanel, ID_Button_Update, Language->ButtonUpdate, wxDefaultPosition, wxSize(140,24));
  ReloadButton = new wxButton( ModMakerPanel, ID_Button_Reload, Language->ButtonReload, wxDefaultPosition, wxSize(140,24));
  updateRow->Add( (wxWindow*) UpdateButton, 0, wxRIGHT, 10);
  updateRow->Add( (wxWindow*) ReloadButton, 0, wxEXPAND, 0);
  ModMakerSizer->Add( updateRow, 0, wxEXPAND, 0);

  ModMakerSizer->AddSpacer(10);

  SavedTexturesSizer = new wxStaticBoxSizer(wxVERTICAL, ModMakerPanel, Language->SavedTexturesHeader);
  SavedTexturesList = new wxListCtrl(ModMakerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
  SavedTexturesList->InsertColumn(0, Language->SavedTexturesHeader);
  SavedTexturesImages = new wxImageList(1, 1, true);
  wxBitmap placeholder(1, 1);
  {
    wxMemoryDC dc(placeholder);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SelectObject(wxNullBitmap);
  }
  SavedTexturesPlaceholderIndex = SavedTexturesImages->Add(placeholder);
  SavedTexturesList->SetImageList(SavedTexturesImages, wxIMAGE_LIST_SMALL);
  SavedTexturesStateImages = new wxImageList(16, 16, true);
  wxBitmap unchecked(16, 16);
  {
    wxMemoryDC dc(unchecked);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    wxRendererNative::Get().DrawCheckBox(ModMakerPanel, dc, wxRect(0, 0, 16, 16), 0);
    dc.SelectObject(wxNullBitmap);
  }
  wxBitmap checked(16, 16);
  {
    wxMemoryDC dc(checked);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    wxRendererNative::Get().DrawCheckBox(ModMakerPanel, dc, wxRect(0, 0, 16, 16), wxCONTROL_CHECKED);
    dc.SelectObject(wxNullBitmap);
  }
  SavedTexturesUncheckedIndex = SavedTexturesStateImages->Add(unchecked);
  SavedTexturesCheckedIndex = SavedTexturesStateImages->Add(checked);
  SavedTexturesList->SetImageList(SavedTexturesStateImages, wxIMAGE_LIST_STATE);
  SavedTexturesSizer->Add(SavedTexturesList, 0, wxEXPAND, 0);
  ModMakerSizer->Add(SavedTexturesSizer, 0, wxEXPAND, 0);

  wxFlexGridSizer *packageSizer = new wxFlexGridSizer(2, 2, 8, 10);
  PackageNameLabel = new wxStaticText(ModMakerPanel, wxID_ANY, Language->PackageNameLabel);
  packageSizer->Add(PackageNameLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
  PackageName = new wxTextCtrl(ModMakerPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
  packageSizer->Add(PackageName, 1, wxEXPAND, 0);
  PackageAuthorLabel = new wxStaticText(ModMakerPanel, wxID_ANY, Language->PackageAuthorLabel);
  packageSizer->Add(PackageAuthorLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
  PackageAuthor = new wxTextCtrl(ModMakerPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
  packageSizer->Add(PackageAuthor, 1, wxEXPAND, 0);
  packageSizer->AddGrowableCol(1, 1);
  ModMakerSizer->Add(packageSizer, 0, wxEXPAND | wxTOP, 8);

  SavePackageButton = new wxButton(ModMakerPanel, wxID_ANY, Language->ButtonSavePackage, wxDefaultPosition, wxSize(160, 28));
  ModMakerSizer->Add(SavePackageButton, 0, wxTOP, 8);

  NumberOfEntry = 0;
  MaxNumberOfEntry = 1024;
  if (GetMemory( CheckBoxes, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  if (GetMemory( CheckBoxHSizers, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return ;}
  if (GetMemory( CheckButtonUp, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  if (GetMemory( CheckButtonDown, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  if (GetMemory( CheckButtonDelete, MaxNumberOfEntry)) {LastError = Language->Error_Memory; return;}
  SavePath->SetValue(Language->TextCtrlSavePath);
  RefreshSavedTextures();

  ModMakerPanel->SetSizer(ModMakerSizer);
  ModMakerPanel->SetScrollRate(0, 20);
  ModMakerSizer->FitInside(ModMakerPanel);

  Notebook->AddPage( LauncherPanel, Language->TabLauncher, true);
  Notebook->AddPage( ModMakerPanel, Language->TabModMaker, false);

  SetSizer(MainSizer);

  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonLaunch, this, LaunchButton->GetId());
  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonLocateExe, this, LocateExeButton->GetId());
  Bind( wxEVT_COMMAND_CHECKBOX_CLICKED, &uMod_GamePage::OnToggleLoadDefaultMods, this, LoadDefaultMods->GetId());
  Bind( wxEVT_COMMAND_BUTTON_CLICKED, &uMod_GamePage::OnButtonSavePackage, this, SavePackageButton->GetId());
  Bind( wxEVT_COMMAND_LIST_ITEM_SELECTED, &uMod_GamePage::OnSavedTextureClick, this, SavedTexturesList->GetId());

  UpdateLaunchState();
  LoadDefaultModsList();
  if (LoadDefaultMods->GetValue()) ApplyDefaultMods();
  else ClearModsList(false);
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
  delete SavedTexturesImages;
  delete SavedTexturesStateImages;
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
  if (DirectoryButton!=NULL) DirectoryButton->Enable(true);
  if (UpdateButton!=NULL) UpdateButton->Enable(enable);
  if (ReloadButton!=NULL) ReloadButton->Enable(enable);
  if (SavePackageButton!=NULL) SavePackageButton->Enable(true);
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
  if (len == 0)
  {
    file.Close();
    wxRemoveFile(DEFAULT_MODS_FILE);
    if (LoadDefaultMods!=NULL) LoadDefaultMods->SetValue(false);
    return 0;
  }

  unsigned char* buffer;
  try {buffer = new unsigned char [len+2];}
  catch (...) {return -1;}

  unsigned int result = file.Read( buffer, len);
  file.Close();
  if (result != len) {delete [] buffer; wxRemoveFile(DEFAULT_MODS_FILE); return -1;}

  wxString content;
  bool looks_utf16 = false;
  if (len % 2 == 0)
  {
    unsigned int zero_high = 0;
    unsigned int pairs = len / 2;
    for (unsigned int i=1; i<len; i+=2) if (buffer[i]==0) zero_high++;
    if (pairs>0 && zero_high > (pairs * 7 / 10)) looks_utf16 = true;
  }

  if (looks_utf16)
  {
    wchar_t *buff = (wchar_t*)buffer;
    len/=2;
    buff[len]=0;
    content = buff;
  }
  else
  {
    buffer[len]=0;
    content = wxString::FromUTF8((const char*)buffer);
    if (content.IsEmpty()) content = (const char*)buffer;
  }
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
      if (len == 0)
      {
        file.Close();
        wxRemoveFile(DEFAULT_MODS_STATE_FILE);
        enabled = (DefaultMods.GetCount()>0);
      }
      else
      {
        unsigned char* buffer;
        try {buffer = new unsigned char [len+2];}
        catch (...) {return -1;}

        unsigned int result = file.Read( buffer, len);
        file.Close();
        if (result != len) {delete [] buffer; wxRemoveFile(DEFAULT_MODS_STATE_FILE); return -1;}

        bool looks_utf16 = false;
        if (len % 2 == 0)
        {
          unsigned int zero_high = 0;
          unsigned int pairs = len / 2;
          for (unsigned int i=1; i<len; i+=2) if (buffer[i]==0) zero_high++;
          if (pairs>0 && zero_high > (pairs * 7 / 10)) looks_utf16 = true;
        }

        wxString content;
        if (looks_utf16)
        {
          wchar_t *buff = (wchar_t*)buffer;
          len/=2;
          buff[len]=0;
          content = buff;
        }
        else
        {
          buffer[len]=0;
          content = wxString::FromUTF8((const char*)buffer);
          if (content.IsEmpty()) content = (const char*)buffer;
        }
        delete [] buffer;
        content.Trim(true);
        content.Trim(false);
        enabled = (!content.IsEmpty() && content[0] == L'1');
      }
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
  SuppressDefaultModsSave = true;
  for (unsigned int i=0; i<DefaultMods.GetCount(); i++)
  {
    const wxString &file_name = DefaultMods[i];
    bool exists = false;
    for (int j=0; j<NumberOfEntry; j++) if (Files[j].CmpNoCase(file_name)==0) {exists = true; break;}
    if (!exists)
    {
      if (AddTextureInternal(file_name, false)==0) added++;
      else LastError.Empty();
    }
  }
  SuppressDefaultModsSave = false;
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

void uMod_GamePage::RefreshSavedTextures(void)
{
  if (SavedTexturesList==NULL || SavedTexturesImages==NULL) return;
  SavedTexturesList->DeleteAllItems();
  SavedTextureFiles.Clear();
  SavedTextureChecked.Clear();

  wxString path = Game.GetSavePath();
  if (path.IsEmpty())
  {
    long item = SavedTexturesList->InsertItem(0, Language->SavedTexturesHint, SavedTexturesPlaceholderIndex);
    SavedTexturesList->SetItemData(item, -1);
    SavedTexturesList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    UpdateSavedTexturesListSize();
    return;
  }

  wxDir dir(path);
  if (!dir.IsOpened())
  {
    long item = SavedTexturesList->InsertItem(0, Language->SavedTexturesEmpty, SavedTexturesPlaceholderIndex);
    SavedTexturesList->SetItemData(item, -1);
    SavedTexturesList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    UpdateSavedTexturesListSize();
    return;
  }

  wxString filename;
  bool cont = dir.GetFirst(&filename, "*.dds", wxDIR_FILES);
  int index = 0;
  while (cont)
  {
    wxFileName full_path(path, filename);
    wxString file_path = full_path.GetFullPath();
    long item = SavedTexturesList->InsertItem(index, filename, SavedTexturesPlaceholderIndex);
    SavedTextureFiles.Add(file_path);
    SavedTexturesList->SetItemData(item, SavedTextureFiles.GetCount() - 1);
    SavedTextureChecked.Add(1);
    int state = SavedTexturesCheckedIndex + 1;
    const long state_mask = 0xF000;
    SavedTexturesList->SetItemState(item, state << 12, state_mask);
    index++;
    cont = dir.GetNext(&filename);
  }

  if (SavedTextureFiles.IsEmpty())
  {
    long item = SavedTexturesList->InsertItem(0, Language->SavedTexturesEmpty, SavedTexturesPlaceholderIndex);
    SavedTexturesList->SetItemData(item, -1);
  }

  SavedTexturesList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
  UpdateSavedTexturesListSize();
}

void uMod_GamePage::UpdateSavedTexturesListSize(void)
{
  if (SavedTexturesList==NULL) return;
  int item_count = SavedTexturesList->GetItemCount();
  if (item_count <= 0) item_count = 1;

  int row_height = SavedTexturesList->GetCharHeight() + 6;
  wxRect item_rect;
  if (SavedTexturesList->GetItemRect(0, item_rect)) row_height = item_rect.GetHeight();

  int header_height = SavedTexturesList->GetCharHeight() + 8;
  int total_height = header_height + (row_height * item_count) + 6;
  SavedTexturesList->SetMinSize(wxSize(-1, total_height));
  SavedTexturesList->SetMaxSize(wxSize(-1, total_height));
  if (ModMakerPanel!=NULL && ModMakerSizer!=NULL)
  {
    ModMakerPanel->Layout();
    ModMakerSizer->FitInside(ModMakerPanel);
  }
}

void uMod_GamePage::ToggleSavedTextureSelection(long item)
{
  if (item < 0 || item >= (long)SavedTextureChecked.GetCount()) return;
  SavedTextureChecked[item] = SavedTextureChecked[item] ? 0 : 1;
  int state = SavedTextureChecked[item] ? SavedTexturesCheckedIndex + 1 : SavedTexturesUncheckedIndex + 1;
  const long state_mask = 0xF000;
  SavedTexturesList->SetItemState(item, state << 12, state_mask);
}

bool uMod_GamePage::IsSavedTextureSelected(long item) const
{
  if (item < 0 || item >= (long)SavedTextureChecked.GetCount()) return false;
  return SavedTextureChecked[item] != 0;
}

void uMod_GamePage::OnSavedTextureClick(wxListEvent &event)
{
  long item = event.GetIndex();
  if (item < 0) return;
  ToggleSavedTextureSelection(item);
}

bool uMod_GamePage::ExtractTextureHash(const wxString &file_name, unsigned long &hash) const
{
  wxFileName name(file_name);
  wxString base = name.GetFullName();
  wxString hash_part = base.AfterLast('_');
  hash_part = hash_part.BeforeLast('.');
  return hash_part.ToULong(&hash, 16);
}

int uMod_GamePage::CreateTpfPackage(const wxString &output_path, const wxArrayString &files, const wxString &name, const wxString &author)
{
  if (files.IsEmpty()) {LastError = Language->Error_NoTexturesSelected; return -1;}

  const unsigned char pw[] = {0x73, 0x2A, 0x63, 0x7D, 0x5F, 0x0A, 0xA6, 0xBD,
     0x7D, 0x65, 0x7E, 0x67, 0x61, 0x2A, 0x7F, 0x7F,
     0x74, 0x61, 0x67, 0x5B, 0x60, 0x70, 0x45, 0x74,
     0x5C, 0x22, 0x74, 0x5D, 0x6E, 0x6A, 0x73, 0x41,
     0x77, 0x6E, 0x46, 0x47, 0x77, 0x49, 0x0C, 0x4B,
     0x46, 0x6F, '\0'};

  wxString temp_path = wxFileName::CreateTempFileName("uMod_tpf");
  if (temp_path.IsEmpty()) {LastError = Language->Error_SaveFile; return -1;}

  HZIP zip_handle = CreateZip(temp_path.wc_str(), reinterpret_cast<const char*>(pw));
  if (zip_handle==NULL)
  {
    wxRemoveFile(temp_path);
    LastError = Language->Error_SaveFile;
    return -1;
  }

  wxString def;
  for (unsigned int i=0; i<files.GetCount(); i++)
  {
    unsigned long hash;
    if (!ExtractTextureHash(files[i], hash))
    {
      CloseZip(zip_handle);
      wxRemoveFile(temp_path);
      LastError << Language->Error_Hash << "\n" << files[i];
      return -1;
    }
    wxFileName file_name(files[i]);
    wxString entry_name = file_name.GetFullName();
    def << wxString::Format("%08X|%s\n", hash, entry_name);
    if (ZipAdd(zip_handle, entry_name.wc_str(), files[i].wc_str()) != ZR_OK)
    {
      CloseZip(zip_handle);
      wxRemoveFile(temp_path);
      LastError = Language->Error_SaveFile;
      return -1;
    }
  }

  wxCharBuffer def_buffer = def.ToUTF8();
  if (ZipAdd(zip_handle, L"texmod.def", (void*)def_buffer.data(), def_buffer.length()) != ZR_OK)
  {
    CloseZip(zip_handle);
    wxRemoveFile(temp_path);
    LastError = Language->Error_SaveFile;
    return -1;
  }

  if (!name.IsEmpty() || !author.IsEmpty())
  {
    wxString comment = name;
    if (!author.IsEmpty())
    {
      if (!comment.IsEmpty()) comment << "\n";
      comment << Language->Author << author;
    }
    wxCharBuffer comment_buffer = comment.ToUTF8();
    ZipAdd(zip_handle, L"Comment.txt", (void*)comment_buffer.data(), comment_buffer.length());
  }

  CloseZip(zip_handle);

  wxFile zip_file;
  if (!zip_file.Access(temp_path, wxFile::read) || !zip_file.Open(temp_path, wxFile::read))
  {
    wxRemoveFile(temp_path);
    LastError = Language->Error_FileOpen;
    return -1;
  }

  unsigned long zip_len = zip_file.Length();
  if (zip_len==0)
  {
    zip_file.Close();
    wxRemoveFile(temp_path);
    LastError = Language->Error_FileRead;
    return -1;
  }

  char *zip_buffer = NULL;
  try {zip_buffer = new char[zip_len];}
  catch (...) {zip_buffer = NULL;}
  if (zip_buffer==NULL)
  {
    zip_file.Close();
    wxRemoveFile(temp_path);
    LastError = Language->Error_Memory;
    return -1;
  }

  unsigned int result = zip_file.Read(zip_buffer, zip_len);
  zip_file.Close();
  wxRemoveFile(temp_path);
  if (result != zip_len)
  {
    delete [] zip_buffer;
    LastError = Language->Error_FileRead;
    return -1;
  }

  wxCharBuffer author_buffer = author.ToUTF8();
  unsigned long author_len = author_buffer.length();
  unsigned long total_len = zip_len + 1 + author_len;
  char *tpf_buffer = NULL;
  try {tpf_buffer = new char[total_len];}
  catch (...) {tpf_buffer = NULL;}
  if (tpf_buffer==NULL)
  {
    delete [] zip_buffer;
    LastError = Language->Error_Memory;
    return -1;
  }

  memcpy(tpf_buffer, zip_buffer, zip_len);
  tpf_buffer[zip_len] = 0;
  if (author_len>0) memcpy(tpf_buffer + zip_len + 1, author_buffer.data(), author_len);
  delete [] zip_buffer;

  unsigned int *buff = (unsigned int*) tpf_buffer;
  unsigned int TPF_XOR = 0x3FA43FA4u;
  unsigned int size = total_len/4u;
  for (unsigned int i=0; i<size; i++) buff[i] ^= TPF_XOR;
  for (unsigned int i=size*4u; i<size*4u+total_len%4u; i++) ((unsigned char*) tpf_buffer)[i]^=(unsigned char) TPF_XOR;

  wxFile output;
  if (!output.Open(output_path, wxFile::write))
  {
    delete [] tpf_buffer;
    LastError = Language->Error_SaveFile;
    return -1;
  }
  if (output.Write(tpf_buffer, total_len) != total_len)
  {
    output.Close();
    delete [] tpf_buffer;
    LastError = Language->Error_SaveFile;
    return -1;
  }
  output.Close();
  delete [] tpf_buffer;
  return 0;
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


int uMod_GamePage::AddTexture( const wxString &file_name)
{
  return AddTextureInternal(file_name, true);
}

int uMod_GamePage::AddTextureInternal( const wxString &file_name, bool update_game)
{
  for (int i=0; i<NumberOfEntry; i++)
  {
    if (Files[i].CmpNoCase(file_name)==0) return 0;
  }
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

  if (!SuppressDefaultModsSave) SaveDefaultModsList();
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
  return 0;
}

void uMod_GamePage::OnButtonSavePackage(wxCommandEvent& WXUNUSED(event))
{
  if (SavedTexturesList==NULL) return;
  wxString save_path = Game.GetSavePath();
  if (save_path.IsEmpty())
  {
    wxMessageBox(Language->Error_NoSavePath, "ERROR", wxOK|wxICON_ERROR);
    return;
  }

  wxArrayString selected_files;
  long count = SavedTexturesList->GetItemCount();
  for (long i=0; i<count; i++)
  {
    if (!IsSavedTextureSelected(i)) continue;
    long data = SavedTexturesList->GetItemData(i);
    if (data>=0 && data < (long)SavedTextureFiles.GetCount())
    {
      selected_files.Add(SavedTextureFiles[(unsigned int)data]);
    }
  }

  if (selected_files.IsEmpty())
  {
    wxMessageBox(Language->Error_NoTexturesSelected, "ERROR", wxOK|wxICON_ERROR);
    return;
  }

  wxString package_name = PackageName != NULL ? PackageName->GetValue() : "";
  package_name.Trim(true);
  package_name.Trim(false);
  wxString author = PackageAuthor != NULL ? PackageAuthor->GetValue() : "";
  author.Trim(true);
  author.Trim(false);

  wxString default_name = package_name;
  if (default_name.IsEmpty()) default_name = "texture_pack";

  wxFileDialog save_dialog(this, Language->SavePackageDialog, save_path, default_name, "tpf files (*.tpf)|*.tpf", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (save_dialog.ShowModal() != wxID_OK) return;
  wxString output_path = save_dialog.GetPath();
  if (!output_path.Lower().EndsWith(".tpf")) output_path << ".tpf";

  LastError.Empty();
  if (CreateTpfPackage(output_path, selected_files, package_name, author))
  {
    wxMessageBox(LastError, "ERROR", wxOK|wxICON_ERROR);
    LastError.Empty();
  }
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
  if (ModsSizer!=NULL && ModsSizer->GetStaticBox()!=NULL)
  {
    ModsSizer->GetStaticBox()->SetLabel( Language->LoadedMods);
  }

  KeyBackButton->SetLabel( Language->KeyBack);
  KeySaveButton->SetLabel( Language->KeySave);
  KeyNextButton->SetLabel( Language->KeyNext);
  FontColour[0]->SetValue( Language->FontColour);
  TextureColour[0]->SetValue( Language->TextureColour);
  SaveAllTextures->SetLabel( Language->CheckBoxSaveAllTextures);
  SaveSingleTexture->SetLabel( Language->CheckBoxSaveSingleTexture);
  LoadDefaultMods->SetLabel( Language->LoadDefaultMods);
  if (SavedTexturesSizer!=NULL && SavedTexturesSizer->GetStaticBox()!=NULL)
  {
    SavedTexturesSizer->GetStaticBox()->SetLabel( Language->SavedTexturesHeader);
  }
  if (SavedTexturesList!=NULL)
  {
    wxListItem item;
    item.SetText(Language->SavedTexturesHeader);
    SavedTexturesList->SetColumn(0, item);
    SavedTexturesList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
  }
  if (PackageNameLabel!=NULL) PackageNameLabel->SetLabel( Language->PackageNameLabel);
  if (PackageAuthorLabel!=NULL) PackageAuthorLabel->SetLabel( Language->PackageAuthorLabel);
  if (SavePackageButton!=NULL) SavePackageButton->SetLabel( Language->ButtonSavePackage);
  wxString temp = Language->TextCtrlSavePath;
  temp << Game.GetSavePath();
  SavePath->SetValue( temp);
  RefreshSavedTextures();
  return 0;
}
