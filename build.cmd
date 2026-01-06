@echo off
setlocal enabledelayedexpansion

set ROOT_DIR=%~dp0

echo Building uMod DX9 DLL...
pushd "%ROOT_DIR%uMod_DX9"
nmake -f makefile.vc
if errorlevel 1 (
  echo DX9 build failed.
  popd
  exit /b 1
)
popd

echo Copying uMod DX9 DLL to GUI bin...
if not exist "%ROOT_DIR%uMod_GUI\bin" (
  mkdir "%ROOT_DIR%uMod_GUI\bin"
)
if exist "%ROOT_DIR%uMod_DX9\bin\uMod_d3d9_DI.dll" (
  copy /Y "%ROOT_DIR%uMod_DX9\bin\uMod_d3d9_DI.dll" "%ROOT_DIR%uMod_GUI\bin\uMod_d3d9_DI.dll"
) else (
  echo DX9 DLL not found in uMod_DX9\bin.
  exit /b 1
)

echo Building uMod GUI...
pushd "%ROOT_DIR%uMod_GUI"
nmake -f makefile.vc BUILD=release
if errorlevel 1 (
  echo GUI build failed.
  popd
  exit /b 1
)
popd

echo Ensuring uMod DX9 DLL is available next to GUI...
if exist "%ROOT_DIR%uMod_DX9\bin\uMod_d3d9_DI.dll" (
  copy /Y "%ROOT_DIR%uMod_DX9\bin\uMod_d3d9_DI.dll" "%ROOT_DIR%uMod_GUI\bin\uMod_d3d9_DI.dll"
) else (
  echo DX9 DLL not found in uMod_DX9\bin.
  exit /b 1
)

echo Launching uMod GUI...
echo Syncing runtime assets into GUI bin...
if exist "%ROOT_DIR%uMod_public\languages" (
  xcopy /E /I /Y "%ROOT_DIR%uMod_public\languages" "%ROOT_DIR%uMod_GUI\bin\languages" >nul
)
for %%F in (README_Deutsch.txt README_English.txt README_French.txt uMod_DI_Games.txt uMod_Settings.txt) do (
  if exist "%ROOT_DIR%uMod_public\%%F" (
    copy /Y "%ROOT_DIR%uMod_public\%%F" "%ROOT_DIR%uMod_GUI\bin\%%F" >nul
  )
)

start "" /D "%ROOT_DIR%uMod_GUI\bin" "%ROOT_DIR%uMod_GUI\bin\uMod.exe"
