@echo off
setlocal
if exist "%~dp0x64\sentype-tsf.dll" regsvr32 /u /s "%~dp0x64\sentype-tsf.dll"
if exist "%~dp0x86\sentype-tsf.dll" %SystemRoot%\SysWOW64\regsvr32 /u /s "%~dp0x86\sentype-tsf.dll"
