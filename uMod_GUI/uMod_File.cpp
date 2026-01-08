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
#include "unzip.h"
#include "zip.h"
#include <cstring>
#include <wx/filename.h>


uMod_File::uMod_File(void)
{
  Loaded=false;
  XORed=false;
  FileInMemory=NULL;
  MemoryLength=0u;
  FileLen=0u;
}

uMod_File::uMod_File(const wxString &file)
{
  Loaded=false;
  XORed=false;
  FileInMemory=NULL;
  MemoryLength=0u;
  FileLen=0u;
  SetFile(file);
}


uMod_File::~uMod_File(void)
{
  if (FileInMemory!=NULL) delete [] FileInMemory;
}

namespace
{
void XorTpfBuffer(char *buffer, unsigned int length)
{
  unsigned int *buff = (unsigned int*) buffer;
  unsigned int TPF_XOR = 0x3FA43FA4u;
  unsigned int size = length / 4u;
  for (unsigned int i=0; i<size; i++) buff[i] ^= TPF_XOR;
  for (unsigned int i=size*4u; i<size*4u+length%4u; i++) ((unsigned char*) buffer)[i]^=(unsigned char) TPF_XOR;
}

wxString ExtractHashFromFilename(const wxString &file_name)
{
  wxString name = file_name.AfterLast('_');
  name = name.BeforeLast('.');
  unsigned long temp_hash = 0;
  if (!name.ToULong(&temp_hash, 16)) return wxString();
  return name;
}
} // namespace


bool uMod_File::FileSupported(void)
{
  wxString file_type = FileName.AfterLast( '.');
  if (file_type == L"zip") return true;
  else if (file_type == L"tpf") return true;
  else if (file_type == L"dds") return true;

  return false;
}


int uMod_File::GetComment( wxString &tool_tip)
{
  wxString file_type = FileName.AfterLast( '.');
  if (file_type == L"zip")
  {
    if (int ret = GetCommentZip( tool_tip)) return ret;
  }
  else if (file_type == L"tpf")
  {
    if (int ret = GetCommentTpf( tool_tip)) return ret;
  }
  else if (file_type == L"dds")
  {
    tool_tip = Language->NoComment;
    return -1;
  }
  return 0;
}

int uMod_File::GetContent( AddTextureClass &tex, bool add)
{
  wxString file_type = FileName.AfterLast( '.');
  if (file_type == L"zip")
  {
    AddZip( tex, add, false);
  }
  else if (file_type == L"tpf")
  {
    AddZip( tex, add, true);
  }
  else if (file_type == L"dds")
  {
    AddFile( tex, add);
  }
  else
  {
    LastError << Language->Error_FileNotSupported;
    LastError << "\n" << FileName;
  }
  if (LastError.Len()>0) return -1;
  else
  {
    if (add) tex.Loaded = true;
    return 0;
  }
}

int uMod_File::ReadFile(void)
{
  if (Loaded) return 0;
  XORed=false;

  wxFile dat;
  if (!dat.Access(FileName, wxFile::read)) {LastError << Language->Error_FileOpen <<"\n" << FileName; return -1;}
  dat.Open(FileName, wxFile::read);
  if (!dat.IsOpened()) {LastError << Language->Error_FileOpen <<"\n" << FileName; return -1;}
  FileLen = dat.Length();
  if (FileLen==0) {LastError << Language->Error_FileOpen <<"\n" << FileName; return -1;}

  if (FileLen>=MemoryLength)
  {
    if (FileInMemory!=NULL) delete [] FileInMemory;
    try {FileInMemory = new char [FileLen+1];}
    catch (...) {FileInMemory=NULL; MemoryLength=0; FileLen=0; LastError << Language->Error_Memory; return -1;}
    MemoryLength = FileLen+1;
  }
  unsigned int result = dat.Read( FileInMemory, FileLen);
  dat.Close();

  if (result != FileLen) {FileLen=0; LastError << Language->Error_FileRead<<"\n" << FileName; return -1;}
  FileInMemory[FileLen]=0;

  Loaded = true;
  return 0;
}



int uMod_File::UnXOR(void)
{
  if (XORed) return 0;
  /*
   *
   * BIG THANKS TO Tonttu
   * (TPFcreate 1.5)
   *
   */
  unsigned int *buff = (unsigned int*) FileInMemory;
  unsigned int TPF_XOR = 0x3FA43FA4u;
  unsigned int size = FileLen/4u;
  for (unsigned int i=0; i<size; i++) buff[i] ^= TPF_XOR;
  for (unsigned int i=size*4u; i<size*4u+FileLen%4u; i++) ((unsigned char*) FileInMemory)[i]^=(unsigned char) TPF_XOR;


  unsigned int pos = FileLen-1;
  while (pos>0u && FileInMemory[pos]) pos--;
  if (pos>0u && pos<FileLen-1) FileLen = pos+1;
  XORed = true;

  /*
   * original code by Tonttu
   * The last bytes are not revealed correctly
  unsigned int j=0;
  while ( j <= result - 4 )
  {
    *( unsigned int* )( &buffer[j] ) ^= TPF_XOR;
    j += 4;
  }

  while ( j < result )
  {
    buffer[j] ^= (unsigned char )( TPF_XOR >> 24 );
    TPF_XOR <<= 4;
    j++;
  }
  */
  return 0;
}



int uMod_File::GetCommentZip( wxString &tool_tip)
{
  if (int ret = ReadFile()) return ret;

  HZIP ZIP_Handle = OpenZip( FileInMemory, FileLen, NULL);
  if (ZIP_Handle==NULL)  {tool_tip = Language->NoComment; LastError << Language->Error_Unzip; return -1;}

  ZIPENTRY ze;
  int index;
  FindZipItem( ZIP_Handle, L"Comment.txt", false, &index, &ze);
  if (index>=0) //if Comment.txt is present in the zip file
  {
    char* comment;
    int len = ze.unc_size;
    try {comment=new char[len+1];}
    catch(...) {tool_tip = Language->NoComment; LastError << Language->Error_Memory; return -1;}
    ZRESULT zr = UnzipItem( ZIP_Handle, index, comment, len);

    if (zr!=ZR_OK && zr!=ZR_MORE) {delete [] comment; tool_tip = Language->NoComment;  LastError << Language->Error_Unzip <<"\nZIP:" << L"Comment.txt"; return -1;}
    comment[len]=0;
    tool_tip = comment;
    delete [] comment;
  }
  else tool_tip = Language->NoComment;

  CloseZip(ZIP_Handle);
  return 0;
}

int uMod_File::GetCommentTpf( wxString &tool_tip)
{
  if (int ret = ReadFile()) return ret;

  UnXOR();
  tool_tip = &FileInMemory[FileLen];
  tool_tip.Prepend( Language->Author);
  return 0;
}

int uMod_File::CreateTpfPackage(const wxString &output_path, const wxArrayString &files, const wxString &author, wxString &error)
{
  if (files.IsEmpty())
  {
    error = Language->Error_NoTexturesSelected;
    return -1;
  }

  wxString temp_path = wxFileName::CreateTempFileName("uMod_Tpf");
  if (temp_path.IsEmpty())
  {
    error = Language->Error_TpfCreate;
    return -1;
  }

  HZIP zip_handle = CreateZip(temp_path.wc_str(), NULL);
  if (zip_handle == NULL)
  {
    error = Language->Error_TpfCreate;
    return -1;
  }

  wxString def_content;
  int count = files.GetCount();
  def_content.Alloc(count * 64);
  for (int i=0; i<count; i++)
  {
    wxFileName file_name(files[i]);
    wxString base_name = file_name.GetFullName();
    wxString hash = ExtractHashFromFilename(base_name);
    if (hash.IsEmpty())
    {
      CloseZip(zip_handle);
      wxRemoveFile(temp_path);
      error = Language->Error_Hash;
      error << "\n" << base_name;
      return -1;
    }
    def_content << hash << "|" << base_name << "\n";
  }

  wxCharBuffer def_buffer = def_content.ToUTF8();
  ZRESULT zr = ZipAdd(zip_handle, L"texmod.def", (void*)def_buffer.data(), (unsigned int)def_buffer.length());
  if (zr != ZR_OK)
  {
    CloseZip(zip_handle);
    wxRemoveFile(temp_path);
    error = Language->Error_TpfCreate;
    return -1;
  }

  for (int i=0; i<count; i++)
  {
    wxFileName file_name(files[i]);
    wxString base_name = file_name.GetFullName();
    if (!file_name.FileExists())
    {
      CloseZip(zip_handle);
      wxRemoveFile(temp_path);
      error = Language->Error_FileOpen;
      error << "\n" << files[i];
      return -1;
    }
    zr = ZipAdd(zip_handle, base_name.wc_str(), files[i].wc_str());
    if (zr != ZR_OK)
    {
      CloseZip(zip_handle);
      wxRemoveFile(temp_path);
      error = Language->Error_TpfCreate;
      return -1;
    }
  }

  zr = CloseZip(zip_handle);
  if (zr != ZR_OK)
  {
    wxRemoveFile(temp_path);
    error = Language->Error_TpfCreate;
    return -1;
  }

  wxFile temp_file;
  if (!temp_file.Open(temp_path, wxFile::read))
  {
    wxRemoveFile(temp_path);
    error = Language->Error_FileOpen;
    error << "\n" << temp_path;
    return -1;
  }

  wxFileOffset zip_len = temp_file.Length();
  if (zip_len <= 0)
  {
    temp_file.Close();
    wxRemoveFile(temp_path);
    error = Language->Error_FileRead;
    error << "\n" << temp_path;
    return -1;
  }

  wxCharBuffer author_buffer = author.ToUTF8();
  unsigned int author_len = (unsigned int)author_buffer.length();
  unsigned int total_len = (unsigned int)zip_len + 1u + author_len + 1u;
  char *output_buffer = NULL;
  try {output_buffer = new char[total_len];}
  catch (...)
  {
    temp_file.Close();
    wxRemoveFile(temp_path);
    error = Language->Error_Memory;
    return -1;
  }

  if (temp_file.Read(output_buffer, zip_len) != zip_len)
  {
    delete [] output_buffer;
    temp_file.Close();
    wxRemoveFile(temp_path);
    error = Language->Error_FileRead;
    error << "\n" << temp_path;
    return -1;
  }
  temp_file.Close();
  wxRemoveFile(temp_path);

  output_buffer[zip_len] = '\0';
  if (author_len > 0)
  {
    memcpy(output_buffer + zip_len + 1u, author_buffer.data(), author_len);
  }
  output_buffer[total_len - 1u] = '\0';

  XorTpfBuffer(output_buffer, total_len);

  wxFile out_file;
  if (!out_file.Open(output_path, wxFile::write))
  {
    out_file.Create(output_path, true);
  }
  if (!out_file.IsOpened())
  {
    delete [] output_buffer;
    error = Language->Error_SaveFile;
    error << "\n" << output_path;
    return -1;
  }

  if (out_file.Write(output_buffer, total_len) != total_len)
  {
    delete [] output_buffer;
    out_file.Close();
    error = Language->Error_SaveFile;
    error << "\n" << output_path;
    return -1;
  }

  delete [] output_buffer;
  out_file.Close();
  return 0;
}









int uMod_File::AddFile( AddTextureClass &tex, bool add)
{
  tex.SetSize(1);
  unsigned long temp_hash;

  wxString name = FileName.AfterLast( '_');
  name = name.BeforeLast( '.');
  if (!name.ToULong( &temp_hash, 16)) {LastError << Language->Error_Hash <<"\n" << FileName << "\n"; return -1;} // return if hash could not be extracted

  if (add)
  {
    if (int ret = ReadFile()) return ret;

    try {tex.Textures[0] = new char[FileLen];}
    catch (...) {tex.Textures[0]=NULL; LastError << Language->Error_Memory; return -1;}
    for (unsigned int i=0; i<FileLen; i++) tex.Textures[0][i] = FileInMemory[i];
    tex.Size[0] = FileLen;
  }
  else {tex.Size[0] = 0; tex.Textures[0] = NULL;}

  tex.Num = 1;
  tex.Hash[0] = temp_hash;
  return 0;
}

int uMod_File::AddZip( AddTextureClass &tex, bool add, bool tpf)
{
  if (int ret = ReadFile()) return ret;

  if (tpf)
  {
    UnXOR();

    return AddContent( NULL, tex, add);
  }
  else
  {
    return AddContent( NULL, tex, add);
  }
}


int uMod_File::AddContent( const char* pw, AddTextureClass &tex, bool add)
{
  // Thats is really nasty code, but atm I am happy that it works. I should try an other unzip api,
  // This one seems to behave very strange.
  // I know, that also a bug in my code could be reason for the crashes, but UnzipItem( ... ) unzipes wrong files
  // and GetZipItem( ... ) returns garbage as file names, and the next call of GetZipItem( ... ) blows up the program.
  //
  // I have commented line 3519 in unzip.cpp.
  // It was stated this is a bug, but it did not solve my problems.
  //
  // closing and reopen the zip handle did the trick.
  //

  HZIP ZIP_Handle = OpenZip( FileInMemory, FileLen, pw);
  if (ZIP_Handle==NULL)  {LastError << Language->Error_Unzip; return -1;}

  ZIPENTRY ze;
  int index;
  FindZipItem( ZIP_Handle, L"texmod.def", false, &index, &ze);
  if (index>=0) //if texmod.def is present in the zip file
  {
    char* def;
    int len = ze.unc_size;
    try {def=new char[len+1];}
    catch(...) {LastError << Language->Error_Memory; return -1;}
    ZRESULT zr = UnzipItem( ZIP_Handle, index, def, len);

    if (zr!=ZR_OK && zr!=ZR_MORE) {delete [] def; return -1;}
    def[len]=0;

    wxStringTokenizer token( def, "\n");

    int num = token.CountTokens();


    tex.SetSize(num);

    unsigned long temp_hash;
    int count = 0;
    wxString entry;
    wxString file;

    for (int i=0; i<num; i++)
    {
      entry = token.GetNextToken();
      file = entry.BeforeFirst( '|');
      if (!file.ToULong( &temp_hash, 16)) {LastError << Language->Error_Hash <<"\nTPF:" << entry << "\n"; continue;}

      file = entry.AfterFirst( '|');
      file.Replace( "\r", "");

      while ( (file[0]=='.' && (file[1]=='/' || file[1]=='\\')) || file[0]=='/' || file[0]=='\\') file.Remove(0,1);

      if (add)
      {
        FindZipItem( ZIP_Handle, file.wc_str(), false, &index, &ze); // look for texture
        if (index>=0)
        {
          try {tex.Textures[count] = new char[ze.unc_size];}
          catch(...)
          {
            tex.Textures[count] = NULL;
            LastError << Language->Error_Memory;
            continue;
          }

          ZRESULT rz = UnzipItem( ZIP_Handle, index, tex.Textures[count], ze.unc_size);
          if (rz!=ZR_OK && rz!=ZR_MORE)
          {
            delete [] tex.Textures[count];
            LastError << Language->Error_Unzip <<"\nTPF:" << file << "\n";
            tex.Textures[count] = NULL;
          }
          else
          {
            tex.Hash[count] = temp_hash;
            tex.Size[count] = ze.unc_size;
            count++;
          }
        }
        else
        {
          LastError << Language->Error_Unzip <<"\nTPF:" << file << "\n";
          CloseZip(ZIP_Handle); //somehow we need to close and to reopen the zip handle, otherwise the program crashes
          ZIP_Handle = OpenZip( FileInMemory, FileLen, pw);
        }
      }
      else
      {
        tex.Hash[count] = temp_hash;
        tex.Size[count] = 0;
        count++;
      }
    }
    delete [] def;
    tex.Num = count;
  }
  else // texmod.def is not present in the zip file
  {
    CloseZip(ZIP_Handle); //somehow we need to close and to reopen the zip handle, otherwise the program crashes
    ZIP_Handle = OpenZip( FileInMemory, FileLen, pw);
    if (ZIP_Handle==NULL)  {LastError << Language->Error_Unzip; return -1;}
    wxString name;
    wxString file;
    GetZipItem( ZIP_Handle, -1, &ze); //ask for number of entries
    int num = ze.index;

    tex.SetSize(num);
    int count = 0;
    unsigned long temp_hash;
    for (int i=0; i<num; i++)
    {
      if (GetZipItem( ZIP_Handle, i, &ze)!=ZR_OK) continue; //ask for name and size
      int len = ze.unc_size;

      char *buffer;
      try {buffer = new char[len+1];}
      catch(...)
      {
        continue;
      }


      ZRESULT rz = UnzipItem( ZIP_Handle, i, buffer, len);
      if (rz!=ZR_OK && rz!=ZR_MORE)
      {
        delete [] buffer;
        buffer = NULL;
        LastError << Language->Error_Unzip <<"\nZIP:" << file << "\n";
      }

      file = ze.name;
      if (file.Len()==0) continue;

      name = file.AfterLast( '.');
      if (name!="dds") continue; //if this is not texture file, continue

      name = file.AfterLast( '_');
      name = name.BeforeLast( '.');

      if (!name.ToULong( &temp_hash, 16)) {LastError << Language->Error_Hash <<"\nZIP:" << file << "\n"; continue;} //if hash could not be extracted

      if (add)
      {
        if (buffer==NULL) continue;

        tex.Textures[count] = buffer;
        tex.Hash[count] = temp_hash;
        tex.Size[count] = len;//ze.unc_size;
        count++;
      }
      else
      {
        if (buffer!=NULL) delete [] buffer;
        tex.Hash[count] = temp_hash;
        tex.Size[count] = 0;
        count++;
      }
    }
    tex.Num = count;
  }

  CloseZip(ZIP_Handle);
  return 0;
}
