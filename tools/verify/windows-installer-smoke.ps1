# Real-Windows smoke test of SenType-<ver>-setup.exe: silent install, reinstall (upgrade path), silent uninstall.
#   pwsh tools/verify/windows-installer-smoke.ps1 -Setup dist\SenType-0.1.202610120-setup.exe -Version 0.1.202610120
# Run by .github/workflows/release.yml on windows-latest (needs admin, which hosted runners have).
param([Parameter(Mandatory)][string]$Setup, [Parameter(Mandatory)][string]$Version)
$ErrorActionPreference = 'Stop'
$clsid = '{3922582A-5FEA-483C-BF9B-BD6D5BE68439}'
$inst = Join-Path ${env:ProgramFiles} 'SenType'
$fail = 0
function Check($ok, $what) { if ($ok) { Write-Host "ok   $what" } else { Write-Host "FAIL $what"; $script:fail++ } }
function Hklm($view) { [Microsoft.Win32.RegistryKey]::OpenBaseKey('LocalMachine', $view) }
function Inproc($view) {   # (default) value of the COM server for this view, or $null
  $k = (Hklm $view).OpenSubKey("Software\Classes\CLSID\$clsid\InprocServer32"); if ($k) { $k.GetValue('') } }
function Tip() { $null -ne (Hklm 'Registry64').OpenSubKey("Software\Microsoft\CTF\TIP\$clsid") }
function Arp() { (Hklm 'Registry64').OpenSubKey('Software\Microsoft\Windows\CurrentVersion\Uninstall\SenType') }
function Run($exe, $argList) { $p = Start-Process $exe -ArgumentList $argList -Wait -PassThru; $p.ExitCode }

function CheckInstalled($label) {
  Check (Test-Path "$inst\x64\sentype-tsf.dll") "$label: x64 DLL in $inst\x64"
  Check (Test-Path "$inst\x86\sentype-tsf.dll") "$label: x86 DLL in $inst\x86"
  Check ((Inproc 'Registry64') -eq "$inst\x64\sentype-tsf.dll") "$label: 64-bit CLSID -> x64 DLL"
  Check ((Inproc 'Registry32') -eq "$inst\x86\sentype-tsf.dll") "$label: 32-bit CLSID (WOW6432Node) -> x86 DLL"
  Check (Tip) "$label: CTF TIP profile key present"
  $a = Arp
  Check ($a -and $a.GetValue('DisplayVersion') -eq $Version) "$label: Add/Remove Programs DisplayVersion = $Version"
}

Check ((Run $Setup '/S') -eq 0) 'silent install exit code 0'
CheckInstalled 'install'
Check ((Run $Setup '/S') -eq 0) 'silent reinstall (upgrade path) exit code 0'
CheckInstalled 'reinstall'
Check ((Run "$inst\uninstall.exe" @('/S', "_?=$inst")) -eq 0) 'silent uninstall exit code 0'
Check (-not (Test-Path "$inst\x64\sentype-tsf.dll") -and -not (Test-Path "$inst\x86\sentype-tsf.dll")) 'uninstall: DLLs removed'
Check ($null -eq (Inproc 'Registry64') -and $null -eq (Inproc 'Registry32')) 'uninstall: CLSID keys removed (both views)'
Check (-not (Tip)) 'uninstall: CTF TIP key removed'
Check ($null -eq (Arp)) 'uninstall: Add/Remove Programs entry removed'
Remove-Item $inst -Recurse -Force -ErrorAction SilentlyContinue   # uninstall.exe stays when run with _?=
if ($fail) { Write-Host "$fail check(s) failed"; exit 1 }
Write-Host 'all checks passed'
