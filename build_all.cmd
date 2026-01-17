@echo on
cd /d C:\Users\Administrator\Desktop\uMod_Source_r44\uMod_Source\uMod_GUI || exit /b 1
if not exist bin mkdir bin
nmake -f makefile.vc BUILD=release || exit /b 1

cd /d C:\Users\Administrator\Desktop\uMod_Source_r44\uMod_Source\uMod_DX9 || exit /b 1
nmake -f makefile.vc || exit /b 1
nmake -f makefile.vc DI=1 || exit /b 1
nmake -f makefile.vc NI=1 || exit /b 1

move /y *.dll ..\uMod_GUI\bin
