@echo off
rem Run as Administrator from the folder holding sentype-tsf.dll.
rem 64-bit Windows: 64-bit apps load the x64 DLL, 32-bit apps the x86 DLL -> register both.
setlocal
if exist "%~dp0x64\sentype-tsf.dll" regsvr32 /s "%~dp0x64\sentype-tsf.dll"
if exist "%~dp0x86\sentype-tsf.dll" %SystemRoot%\SysWOW64\regsvr32 /s "%~dp0x86\sentype-tsf.dll"
echo Done. Settings ^> Time ^& language ^> Language ^& region ^> Vietnamese ^> Language options ^> Add a keyboard ^> SenType.
