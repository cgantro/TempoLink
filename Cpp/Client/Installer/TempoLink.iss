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
; The source path will be passed via command line ISCC /DSourceExe=... 
; We also copy .env.deploy as .env so that it runs in deploy mode by default
Source: "{#SourceExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceEnv}"; DestDir: "{app}"; DestName: ".env.deploy"; Flags: ignoreversion

[Icons]
Name: "{group}\TempoLink"; Filename: "{app}\tempolink_juce_client.exe"
Name: "{autodesktop}\TempoLink"; Filename: "{app}\tempolink_juce_client.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Registry]
; tempolink:// Custom URI Protocol Handler
Root: HKCR; Subkey: "tempolink"; ValueType: string; ValueName: ""; ValueData: "URL:TempoLink Protocol"; Flags: uninsdeletekey
Root: HKCR; Subkey: "tempolink"; ValueType: string; ValueName: "URL Protocol"; ValueData: ""; Flags: uninsdeletekey
Root: HKCR; Subkey: "tempolink\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\tempolink_juce_client.exe,0"; Flags: uninsdeletekey
Root: HKCR; Subkey: "tempolink\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\tempolink_juce_client.exe"" ""%1"""; Flags: uninsdeletekey

[Run]
Filename: "{app}\tempolink_juce_client.exe"; Description: "Launch TempoLink"; Flags: nowait postinstall skipifsilent
