@echo off
setlocal enabledelayedexpansion

set ROOT_DIR=%~dp0

echo Cleaning uMod DX9 DLL output...
if exist "%ROOT_DIR%uMod_DX9\bin" (
  rmdir /S /Q "%ROOT_DIR%uMod_DX9\bin"
)
if exist "%ROOT_DIR%uMod_DX9\obj" (
  rmdir /S /Q "%ROOT_DIR%uMod_DX9\obj"
)

echo Cleaning uMod GUI output...
if exist "%ROOT_DIR%uMod_GUI\bin" (
  rmdir /S /Q "%ROOT_DIR%uMod_GUI\bin"
)
if exist "%ROOT_DIR%uMod_GUI\obj" (
  rmdir /S /Q "%ROOT_DIR%uMod_GUI\obj"
)

echo Building uMod DX9 DLL...
pushd "%ROOT_DIR%uMod_DX9"
nmake -f makefile.vc clean
if errorlevel 1 (
  echo DX9 clean failed.
  popd
  exit /b 1
)
nmake -f makefile.vc /a
if errorlevel 1 (
  echo DX9 build failed.
  popd
  exit /b 1
)
popd

echo Copying uMod DX9 DLL to GUI bin...
if exist "%ROOT_DIR%uMod_DX9\bin\uMod_d3d9_DI.dll" (
  copy /Y "%ROOT_DIR%uMod_DX9\bin\uMod_d3d9_DI.dll" "%ROOT_DIR%uMod_GUI\bin\uMod_d3d9_DI.dll"
) else (
  echo DX9 DLL not found in uMod_DX9\bin.
  exit /b 1
)

echo Building uMod GUI...
pushd "%ROOT_DIR%uMod_GUI"
nmake -f makefile.vc clean
if errorlevel 1 (
  echo GUI clean failed.
  popd
  exit /b 1
)
nmake -f makefile.vc BUILD=release /a
if errorlevel 1 (
  echo GUI build failed.
  popd
  exit /b 1
)
popd

echo Launching uMod GUI...
start "" "%ROOT_DIR%uMod_GUI\bin\uMod.exe"
