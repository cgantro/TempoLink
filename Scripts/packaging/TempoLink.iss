#define MyAppName "TempoLink"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "TempoLink Team"
; CLI에서 주입받는 ExeName 매크로 사용
#define MyAppExeName ExeName

[Setup]
AppId={A1B2C3D4-E5F6-7890-1234-567890ABCDEF}
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

; 설치 프로그램(.exe) 자체의 아이콘 (프로젝트 루트의 asset 폴더 내 ico 파일 참조)
SetupIconFile=..\..\asset\logo.ico
; 제어판의 '프로그램 추가/제거' 목록에 표시될 아이콘 (설치된 앱의 아이콘 사용)
UninstallDisplayIcon={app}\{#MyAppExeName}

; 라이선스 동의 창 활성화 (동일 폴더 내 License.txt 또는 .rtf 파일 필요)
LicenseFile=License.txt

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; CLI에서 주입받는 AppDir 매크로를 사용하여 Staging 폴더의 모든 파일을 복사
Source: "{#AppDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent