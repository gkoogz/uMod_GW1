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


uMod_Settings::uMod_Settings(void)
{
  XSize = 600;
  YSize = 400;
  XPos = -1;
  YPos = -1;
  Language = "English";
}

uMod_Settings::uMod_Settings(uMod_Settings &set)
{
  XSize = set.XSize;
  YSize = set.YSize;
  XPos = set.XPos;
  YPos = set.YPos;
  Language = set.Language;
}


static wxString GetSettingsPath(void)
{
  wxString dir = wxStandardPaths::Get().GetUserLocalDataDir();
  if (!dir.IsEmpty())
  {
    wxFileName::Mkdir(dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    wxFileName file_name(dir, "uMod_Settings.txt");
    return file_name.GetFullPath();
  }
  return "uMod_Settings.txt";
}

int uMod_Settings::Load(void)
{
  wxFile file;

  wxString settings_path = GetSettingsPath();
  if (!file.Access(settings_path, wxFile::read)) {return 0;}
  file.Open(settings_path, wxFile::read);
  if (!file.IsOpened()) return -1;

  unsigned len = file.Length();
  if (len == 0 || (len % 2) != 0) {return -1;}

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

  int num = token.CountTokens();
  wxString line;
  wxString command;
  wxString value;

  for (int i=0; i<num; i++)
  {
    line = token.GetNextToken();
    command = line.BeforeFirst(':');
    value = line.AfterFirst(':');
    value.Replace( "\r", "");
    value.Replace( "\n", "");
    if (command == "Lang") Language = value;
    else if (command == "x_size")
    {
      long x;
      if (value.ToLong( &x)) XSize=x;
    }
    else if (command == "y_size")
    {
      long y;
      if (value.ToLong( &y)) YSize=y;
    }
    else if (command == "x_pos")
    {
      long x;
      if (value.ToLong( &x)) XPos=x;
    }
    else if (command == "y_pos")
    {
      long y;
      if (value.ToLong( &y)) YPos=y;
    }
  }

  return 0;
}

int uMod_Settings::Save(void)
{
  wxString settings_path = GetSettingsPath();
  wxFileName settings_file(settings_path);
  wxFileName::Mkdir(settings_file.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  wxString temp_path = settings_path + ".tmp";
  wxFile file;
  file.Open(temp_path, wxFile::write);
  if (!file.IsOpened()) return -1;

  wxString content;

  content = "Lang:";
  content << Language << "\n";
  file.Write( content.wc_str(), content.Len()*2);

  content.Printf("x_size:%d\n", XSize);
  file.Write( content.wc_str(), content.Len()*2);

  content.Printf("y_size:%d\n", YSize);
  file.Write( content.wc_str(), content.Len()*2);

  content.Printf("x_pos:%d\n", XPos);
  file.Write( content.wc_str(), content.Len()*2);

  content.Printf("y_pos:%d\n", YPos);
  file.Write( content.wc_str(), content.Len()*2);

  file.Close();
  wxRenameFile(temp_path, settings_path, true);

  return 0;
}
