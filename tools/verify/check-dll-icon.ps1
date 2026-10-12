# Fails unless the DLL carries the SenType icon (RT_GROUP_ICON id 101, see frontends/windows/res/sentype.rc).
#   tools/verify/check-dll-icon.ps1 build\frontends\windows\Release\sentype-tsf.dll
param([Parameter(Mandatory)][string]$Dll)
$ErrorActionPreference = 'Stop'
Add-Type -Namespace W -Name N -MemberDefinition @'
[DllImport("kernel32", SetLastError=true, CharSet=CharSet.Unicode)] public static extern IntPtr LoadLibraryExW(string f, IntPtr h, uint fl);
[DllImport("kernel32", SetLastError=true, CharSet=CharSet.Unicode)] public static extern IntPtr FindResourceW(IntPtr m, IntPtr name, IntPtr type);
[DllImport("kernel32")] public static extern bool FreeLibrary(IntPtr m);
'@
$h = [W.N]::LoadLibraryExW((Resolve-Path $Dll).Path, [IntPtr]::Zero, 0x2)  # LOAD_LIBRARY_AS_DATAFILE
if ($h -eq [IntPtr]::Zero) { throw "cannot load $Dll as data file" }
$group = [W.N]::FindResourceW($h, [IntPtr]101, [IntPtr]14)  # RT_GROUP_ICON
$icon = [W.N]::FindResourceW($h, [IntPtr]1, [IntPtr]3)       # RT_ICON (first image of the group)
[void][W.N]::FreeLibrary($h)
if ($group -eq [IntPtr]::Zero -or $icon -eq [IntPtr]::Zero) { throw "icon resource 101 missing in $Dll" }
Write-Host "icon resource 101 present in $Dll"
