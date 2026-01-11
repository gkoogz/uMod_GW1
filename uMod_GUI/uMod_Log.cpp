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

#include "uMod_Log.h"

#include <wx/datetime.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

static wxString GetLogPath(void)
{
  wxFileName exe_path(wxStandardPaths::Get().GetExecutablePath());
  return exe_path.GetPathWithSep() + "uMod_Reforged.log";
}

wxString FormatWindowsError(unsigned long error_code)
{
  if (error_code == 0) return "Unknown error.";
  wxString message = wxSysErrorMsg(error_code);
  if (message.IsEmpty()) return "Unknown error.";
  message.Trim(true);
  message.Trim(false);
  return message;
}

wxString FormatWaitResult(unsigned long wait_result)
{
  switch (wait_result)
  {
    case 0x0: return "WAIT_OBJECT_0";
    case 0x102: return "WAIT_TIMEOUT";
    case 0x80: return "WAIT_ABANDONED";
    case 0xFFFFFFFF: return "WAIT_FAILED";
    default: return wxString::Format("WAIT_%lu", wait_result);
  }
}

void AppendToLog(const wxString &line)
{
  wxString message = wxDateTime::Now().FormatISOCombined(' ');
  message << " " << line << "\n";
  wxFile file;
  if (!file.Open(GetLogPath(), wxFile::write_append)) file.Open(GetLogPath(), wxFile::write);
  if (!file.IsOpened()) return;
  file.Write(message);
  file.Close();
}
