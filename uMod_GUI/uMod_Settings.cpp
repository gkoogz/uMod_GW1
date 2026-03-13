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
#include <wx/display.h>
#include <wx/filename.h>

namespace {
const int kDefaultWidth = 500;
const int kDefaultHeight = 400;
const int kDefaultPosX = 400;
const int kDefaultPosY = 400;
const int kMinWidth = 200;
const int kMinHeight = 200;

static void ResetWindowSettings(uMod_Settings &settings)
{
  settings.XSize = kDefaultWidth;
  settings.YSize = kDefaultHeight;
  settings.XPos = kDefaultPosX;
  settings.YPos = kDefaultPosY;
}

static void ClampWindowSettingsToDisplay(uMod_Settings &settings)
{
  if (settings.XSize < kMinWidth) settings.XSize = kMinWidth;
  if (settings.YSize < kMinHeight) settings.YSize = kMinHeight;
  if (settings.XSize > 4000) settings.XSize = 4000;
  if (settings.YSize > 4000) settings.YSize = 4000;
  if (settings.XPos < -32000) settings.XPos = -32000;
  if (settings.YPos < -32000) settings.YPos = -32000;
  if (settings.XPos > 32000) settings.XPos = 32000;
  if (settings.YPos > 32000) settings.YPos = 32000;

  int display_count = wxDisplay::GetCount();
  if (display_count <= 0) return;

  wxRect frame_rect(settings.XPos, settings.YPos, settings.XSize, settings.YSize);
  int best_display = wxNOT_FOUND;
  int best_area = -1;
  for (int i = 0; i < display_count; ++i)
  {
    wxDisplay display(i);
    if (!display.IsOk()) continue;
    wxRect geometry = display.GetClientArea();
    wxRect intersection = geometry.Intersect(frame_rect);
    int area = intersection.GetWidth() * intersection.GetHeight();
    if (area > best_area)
    {
      best_area = area;
      best_display = i;
    }
  }

  if (best_display == wxNOT_FOUND) best_display = 0;
  wxDisplay display(best_display);
  if (!display.IsOk()) return;

  wxRect geometry = display.GetClientArea();
  bool suspicious_restore = (best_area <= 0);
  suspicious_restore = suspicious_restore || (settings.XSize >= (geometry.GetWidth() * 9) / 10);
  suspicious_restore = suspicious_restore || (settings.YSize >= (geometry.GetHeight() * 9) / 10);

  if (suspicious_restore)
  {
    settings.XSize = kDefaultWidth;
    settings.YSize = kDefaultHeight;
    settings.XPos = geometry.GetX() + ((geometry.GetWidth() - settings.XSize) / 2);
    settings.YPos = geometry.GetY() + ((geometry.GetHeight() - settings.YSize) / 3);
  }

  if (settings.XSize > geometry.GetWidth()) settings.XSize = geometry.GetWidth();
  if (settings.YSize > geometry.GetHeight()) settings.YSize = geometry.GetHeight();
  if (settings.XSize < kMinWidth) settings.XSize = kMinWidth;
  if (settings.YSize < kMinHeight) settings.YSize = kMinHeight;

  int max_x = geometry.GetRight() - settings.XSize + 1;
  int max_y = geometry.GetBottom() - settings.YSize + 1;
  if (settings.XPos < geometry.GetX()) settings.XPos = geometry.GetX();
  if (settings.YPos < geometry.GetY()) settings.YPos = geometry.GetY();
  if (settings.XPos > max_x) settings.XPos = max_x;
  if (settings.YPos > max_y) settings.YPos = max_y;
}
}

uMod_Settings::uMod_Settings(void)
{
  ResetWindowSettings(*this);
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
  return GetReforgedAppDataPath("uMod_Reforged_Settings.txt");
}

int uMod_Settings::Load(void)
{
  wxFile file;
  auto reset_defaults = [this]() {
    ResetWindowSettings(*this);
    Language = "English";
  };

  wxString settings_path = GetSettingsPath();
  if (!file.Access(settings_path, wxFile::read))
  {
    reset_defaults();
    Save();
    return 0;
  }
  file.Open(settings_path, wxFile::read);
  if (!file.IsOpened())
  {
    reset_defaults();
    Save();
    return 0;
  }

  unsigned len = file.Length();
  if (len == 0 || (len % 2) != 0)
  {
    file.Close();
    wxRemoveFile(settings_path);
    reset_defaults();
    Save();
    return 0;
  }

  unsigned char* buffer;
  try {buffer = new unsigned char [len+2];}
  catch (...) {return -1;}

  unsigned int result = file.Read( buffer, len);
  file.Close();

  if (result != len)
  {
    delete [] buffer;
    wxRemoveFile(settings_path);
    reset_defaults();
    Save();
    return 0;
  }

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

  if (Language.IsEmpty()) Language = "English";
  ClampWindowSettingsToDisplay(*this);
  return 0;
}

int uMod_Settings::Save(void)
{
  ClampWindowSettingsToDisplay(*this);

  wxString settings_path = GetSettingsPath();
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
