[Setup]
AppName=TempoLink
AppVersion=0.1.0
AppPublisher=TempoLink Team
DefaultDirName={autopf}\TempoLink
DefaultGroupName=TempoLink
OutputBaseFilename=TempoLink-Installer
Compression=lzma
SolidCompression=yes
PrivilegesRequired=lowest

[Files]
; The source directory will be passed via command line ISCC /DAppDir=...
Source: "{#AppDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\TempoLink"; Filename: "{app}\{#ExeName}"
Name: "{autodesktop}\TempoLink"; Filename: "{app}\{#ExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Registry]
; tempolink:// Custom URI Protocol Handler
Root: HKCR; Subkey: "tempolink"; ValueType: string; ValueName: ""; ValueData: "URL:TempoLink Protocol"; Flags: uninsdeletekey
Root: HKCR; Subkey: "tempolink"; ValueType: string; ValueName: "URL Protocol"; ValueData: ""; Flags: uninsdeletekey
Root: HKCR; Subkey: "tempolink\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#ExeName},0"; Flags: uninsdeletekey
Root: HKCR; Subkey: "tempolink\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#ExeName}"" ""%1"""; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\tempolink"; ValueType: string; ValueName: ""; ValueData: "URL:TempoLink Protocol"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\tempolink"; ValueType: string; ValueName: "URL Protocol"; ValueData: ""; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\tempolink\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#ExeName},0"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\tempolink\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#ExeName}"" ""%1"""; Flags: uninsdeletekey

[Run]
Filename: "{app}\{#ExeName}"; Description: "Launch TempoLink"; Flags: nowait postinstall skipifsilent
