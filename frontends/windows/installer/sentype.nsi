; SenType Windows installer (NSIS 3). Installs the TSF DLLs for both bitnesses and registers them.
;   makensis /DVERSION=0.1.202610120 /DVERSION_BASE=0.1 /DDLL_X64=<x64 dll> /DDLL_X86=<x86 dll> \
;            /DOUTFILE=SenType-0.1.202610120-setup.exe sentype.nsi
; Layout:  $PROGRAMFILES64\SenType\x64\sentype-tsf.dll   (64-bit apps)
;          $PROGRAMFILES64\SenType\x86\sentype-tsf.dll   (32-bit apps, registered with SysWOW64 regsvr32)
Unicode true
!include MUI2.nsh
!include x64.nsh
!include LogicLib.nsh
!ifndef VERSION
  !error "pass /DVERSION=<full version> /DVERSION_BASE=<major.minor> /DDLL_X64=... /DDLL_X86=... /DOUTFILE=..."
!endif

!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\SenType"
!define APP_KEY "Software\SenType"
!define DLL "sentype-tsf.dll"

Name "SenType ${VERSION}"
OutFile "${OUTFILE}"
InstallDir "$PROGRAMFILES64\SenType"
InstallDirRegKey HKLM "${APP_KEY}" "InstallDir"   ; upgrades default to the previous location
RequestExecutionLevel admin
SetCompressor /SOLID lzma
BrandingText "SenType ${VERSION}"

; Windows file versions are 4 x 16-bit numbers: the date part does not fit, so major.minor.0.0 here
; and the full tag version in the string fields.
VIProductVersion "${VERSION_BASE}.0.0"
VIAddVersionKey "ProductName" "SenType"
VIAddVersionKey "FileDescription" "SenType Vietnamese input method installer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "LegalCopyright" "SenType"

!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TEXT "SenType is installed. Add it under Settings > Time & language > Language & region > Vietnamese > Language options > Add a keyboard > SenType.$\r$\n$\r$\nUpgrading? Sign out and in again so running apps pick up the new DLL."
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Var RegFailed

; $0 = regsvr32 exit code (a macro, not a Function: the uninstaller needs it too)
!macro CHECK_REG
  ${If} $0 != 0
    StrCpy $RegFailed 1
    DetailPrint "regsvr32 failed with exit code $0"
  ${EndIf}
!macroend

; regsvr32 per bitness. ${FLAG} is "" (register) or "/u". Needs $INSTDIR to point at the files.
!macro REGISTER_ALL FLAG
  ${If} ${RunningX64}
    ${If} ${FileExists} "$INSTDIR\x64\${DLL}"
      ${DisableX64FSRedirection}   ; this 32-bit installer must reach the real System32 (64-bit regsvr32)
      ExecWait '"$SYSDIR\regsvr32.exe" /s ${FLAG} "$INSTDIR\x64\${DLL}"' $0
      ${EnableX64FSRedirection}
      !insertmacro CHECK_REG
    ${EndIf}
    ${If} ${FileExists} "$INSTDIR\x86\${DLL}"
      ExecWait '"$WINDIR\SysWOW64\regsvr32.exe" /s ${FLAG} "$INSTDIR\x86\${DLL}"' $0
      !insertmacro CHECK_REG
    ${EndIf}
  ${ElseIf} ${FileExists} "$INSTDIR\x86\${DLL}"
    ExecWait '"$SYSDIR\regsvr32.exe" /s ${FLAG} "$INSTDIR\x86\${DLL}"' $0
    !insertmacro CHECK_REG
  ${EndIf}
!macroend

; Replace a DLL that a running app may still have loaded: a loaded DLL can be renamed but not deleted.
!macro PUT_DLL SUBDIR SRC
  CreateDirectory "$INSTDIR\${SUBDIR}"
  SetOutPath "$INSTDIR\${SUBDIR}"
  Delete "$INSTDIR\${SUBDIR}\${DLL}"
  ${If} ${FileExists} "$INSTDIR\${SUBDIR}\${DLL}"
    Delete "$INSTDIR\${SUBDIR}\${DLL}.old"
    Rename "$INSTDIR\${SUBDIR}\${DLL}" "$INSTDIR\${SUBDIR}\${DLL}.old"
    Delete /REBOOTOK "$INSTDIR\${SUBDIR}\${DLL}.old"
    SetRebootFlag true
  ${EndIf}
  File "/oname=${DLL}" "${SRC}"
!macroend

Function .onInit
  ${If} ${RunningX64}
    SetRegView 64
    ReadRegStr $R0 HKLM "${APP_KEY}" "InstallDir"   ; re-read in the 64-bit view
    ${If} $R0 != ""
      StrCpy $INSTDIR $R0
    ${EndIf}
  ${Else}
    StrCpy $INSTDIR "$PROGRAMFILES\SenType"
  ${EndIf}
FunctionEnd

Section "SenType" SecMain
  StrCpy $RegFailed 0
  ; Upgrade over an existing install: unregister the old DLLs from where they are, then replace them.
  ReadRegStr $R1 HKLM "${APP_KEY}" "InstallDir"
  ${If} $R1 != ""
    StrCpy $R2 $INSTDIR
    StrCpy $INSTDIR $R1
    !insertmacro REGISTER_ALL "/u"
    StrCpy $INSTDIR $R2
  ${EndIf}

  !insertmacro PUT_DLL "x86" "${DLL_X86}"
  ${If} ${RunningX64}
    !insertmacro PUT_DLL "x64" "${DLL_X64}"
  ${EndIf}
  WriteUninstaller "$INSTDIR\uninstall.exe"

  WriteRegStr HKLM "${APP_KEY}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "${APP_KEY}" "Version" "${VERSION}"
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayName" "SenType"
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "${UNINST_KEY}" "Publisher" "SenType"
  WriteRegStr HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\x86\${DLL},-101"   ; icon resource 101 of the DLL (always installed)
  WriteRegStr HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "${UNINST_KEY}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair" 1

  !insertmacro REGISTER_ALL ""
  ${If} $RegFailed = 1
    MessageBox MB_OK|MB_ICONSTOP "SenType DLL registration failed (regsvr32). See the details list." /SD IDOK
    SetErrorLevel 2
    Abort
  ${EndIf}
SectionEnd

Section "Uninstall"
  StrCpy $RegFailed 0
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
  !insertmacro REGISTER_ALL "/u"
  Delete /REBOOTOK "$INSTDIR\x64\${DLL}"
  Delete /REBOOTOK "$INSTDIR\x86\${DLL}"
  Delete "$INSTDIR\x64\${DLL}.old"
  Delete "$INSTDIR\x86\${DLL}.old"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR\x64"
  RMDir "$INSTDIR\x86"
  RMDir "$INSTDIR"
  DeleteRegKey HKLM "${UNINST_KEY}"
  DeleteRegKey HKLM "${APP_KEY}"
  ; HKCU\Software\SenType (user settings: Method, TonePlacement) is kept on purpose.
SectionEnd
