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

echo Building uMod GUI...
pushd "%ROOT_DIR%uMod_GUI"
nmake -f makefile.vc BUILD=release
if errorlevel 1 (
  echo GUI build failed.
  popd
  exit /b 1
)
popd

echo Launching uMod GUI...
start "" "%ROOT_DIR%uMod_GUI\bin\uMod.exe"
