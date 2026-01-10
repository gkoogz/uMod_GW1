@echo off
setlocal enabledelayedexpansion

set ROOT_DIR=%~dp0

echo Cleaning uMod_Reforged DX9 bin...
if exist "%ROOT_DIR%uMod_DX9\bin" rmdir /s /q "%ROOT_DIR%uMod_DX9\bin"
if not exist "%ROOT_DIR%uMod_DX9\bin" mkdir "%ROOT_DIR%uMod_DX9\bin"

echo Building uMod_Reforged DX9 DLL...
pushd "%ROOT_DIR%uMod_DX9"
nmake -f makefile.vc
if errorlevel 1 (
  echo DX9 build failed.
  popd
  exit /b 1
)
popd

echo Cleaning uMod_Reforged GUI bin...
if exist "%ROOT_DIR%uMod_GUI\bin" rmdir /s /q "%ROOT_DIR%uMod_GUI\bin"
if not exist "%ROOT_DIR%uMod_GUI\bin" mkdir "%ROOT_DIR%uMod_GUI\bin"

echo Copying uMod_Reforged DX9 DLL to GUI bin...
if exist "%ROOT_DIR%uMod_DX9\bin\uMod_Reforged_d3d9_DI.dll" (
  copy /Y "%ROOT_DIR%uMod_DX9\bin\uMod_Reforged_d3d9_DI.dll" "%ROOT_DIR%uMod_GUI\bin\uMod_Reforged_d3d9_DI.dll"
) else (
  echo DX9 DLL not found in uMod_DX9\bin.
  exit /b 1
)

echo Building uMod_Reforged GUI...
pushd "%ROOT_DIR%uMod_GUI"
nmake -f makefile.vc BUILD=release
if errorlevel 1 (
  echo GUI build failed.
  popd
  exit /b 1
)
popd

echo Launching uMod_Reforged GUI...
start "" "%ROOT_DIR%uMod_GUI\bin\uMod_Reforged.exe"
