#define MyAppName "TempoLink"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "TempoLink Team"
; CLI에서 주입받는 ExeName 매크로 사용
#define MyAppExeName ExeName

[Setup]
AppId={{A1B2C3D4-E5F6-7890-1234-567890ABCDEF}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputBaseFilename=TempoLink-Installer
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

; 커스텀 .ico가 없으면 기본 Inno 아이콘 사용
; SetupIconFile=..\..\Assets\Logos\logo.png
; 제어판의 '프로그램 추가/제거' 목록에 표시될 아이콘 (설치된 앱의 아이콘 사용)
UninstallDisplayIcon={app}\{#MyAppExeName}

; 라이선스 동의 창 활성화 (동일 폴더 내 License.txt 또는 .rtf 파일 필요)
#ifexist "License.txt"
LicenseFile=License.txt
#endif

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Whitelist 방식으로 배포 파일만 명시적으로 포함
Source: "{#AppDir}\*.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppDir}\.env.deploy"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
