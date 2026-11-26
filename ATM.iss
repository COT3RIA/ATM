; ATM by COTERIA - Windows installer (built by GitHub Actions with Inno Setup)
; Expected defines (passed on the command line): VstDir, StandaloneDir, AppVer

#ifndef AppVer
  #define AppVer "1.0.0"
#endif

[Setup]
AppId={{6F2B0D3A-8C41-4E5B-9A57-C07E41A7A001}
AppName=ATM
AppVersion={#AppVer}
AppPublisher=COTERIA
DefaultDirName={autopf}\COTERIA\ATM
DefaultGroupName=COTERIA
DisableProgramGroupPage=yes
DisableDirPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=output
OutputBaseFilename=ATM-Setup-Windows
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=ATM (COTERIA)
; COTERIA logo (files in assets\)
#ifexist "assets\logo.ico"
SetupIconFile=assets\logo.ico
WizardImageFile=assets\wizard_large.bmp
WizardSmallImageFile=assets\wizard_small.bmp
#endif

[Types]
Name: "full"; Description: "Full installation (VST3 plugin + standalone app)"
Name: "vst3only"; Description: "VST3 plugin only"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "vst3"; Description: "ATM VST3 plugin (for FL Studio, Cubase, Ableton, Reaper...)"; Types: full vst3only custom; Flags: fixed
Name: "standalone"; Description: "ATM standalone application"; Types: full custom

[Files]
Source: "{#VstDir}\ATM.vst3\*"; DestDir: "{commonpf64}\Common Files\VST3\ATM.vst3"; Components: vst3; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#StandaloneDir}\ATM.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion
Source: "docs\ATM_Guide_RU.pdf"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "docs\ATM_Guide_EN.pdf"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\ATM"; Filename: "{app}\ATM.exe"; Components: standalone
Name: "{group}\ATM User Guide (RU)"; Filename: "{app}\ATM_Guide_RU.pdf"; Check: FileExists(ExpandConstant('{app}\ATM_Guide_RU.pdf'))
Name: "{group}\ATM User Guide (EN)"; Filename: "{app}\ATM_Guide_EN.pdf"; Check: FileExists(ExpandConstant('{app}\ATM_Guide_EN.pdf'))
Name: "{group}\Uninstall ATM"; Filename: "{uninstallexe}"

[InstallDelete]
; the first test builds were called "Audio To MIDI"
Type: filesandordirs; Name: "{commonpf64}\Common Files\VST3\Audio To MIDI.vst3"

[Run]
Filename: "{app}\ATM.exe"; Description: "Start ATM"; Flags: nowait postinstall skipifsilent unchecked; Components: standalone

[Messages]
FinishedLabel=ATM has been installed. Rescan your plugins in the DAW (in FL Studio: Plugin Manager -> Find installed plugins). ATM is an effect: add it to a mixer channel insert slot.
