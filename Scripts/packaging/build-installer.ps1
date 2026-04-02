# 인코딩을 UTF-8로 설정하여 한글 깨짐 방지
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "=== TempoLink Windows Installer Packaging ===" -ForegroundColor Cyan

# 1. Inno Setup 컴파일러(iscc.exe) 경로 찾기
$ISCC = Get-Command iscc -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
if (-not $ISCC) {
    $ISCC = "${env:ProgramFiles(x86)}\Inno Setup 6\iscc.exe"
}

if (-not (Test-Path $ISCC)) {
    Write-Error "Inno Setup 6 컴파일러를 찾을 수 없습니다: $ISCC"
    Write-Error "공식 홈페이지(https://jrsoftware.org/isinfo.php)에서 설치 후 다시 시도해주세요."
    exit 1
}

$StagingDir = "$PWD\staging-win"

# 2. CMake install 실행 (실행 파일 및 의존성 DLL 자동 수집)
Write-Host "`n[1/3] CMake Install 실행 중 (의존성 수집)..." -ForegroundColor Yellow
cmake --install out/build/juce-client-debug --prefix "$StagingDir" --config Debug

# 3. 환경변수 파일 복사
Write-Host "`n[2/3] .env.deploy 파일 복사 중..." -ForegroundColor Yellow
if (Test-Path "$PWD\.env.deploy") {
    Copy-Item "$PWD\.env.deploy" "$StagingDir\.env.deploy" -Force
} else {
    Write-Warning "프로젝트 루트에 .env.deploy 파일이 없습니다. 임시 더미 파일을 생성합니다."
    "TEST_ENV=1" | Out-File -FilePath "$StagingDir\.env.deploy" -Encoding utf8
}

# 4. 스테이징된 파일 목록 출력
Write-Host "`n=== Staged files for Inno Setup ===" -ForegroundColor Green
Get-ChildItem "$StagingDir" | Format-Table Name, Length, LastWriteTime

# 5. Inno Setup 패키징 실행
Write-Host "`n[3/3] Inno Setup 패키징 실행 중..." -ForegroundColor Yellow
$OutDir = "$PWD\installer-output"
& $ISCC "/O$OutDir" "/DAppDir=$StagingDir" "/DExeName=TempoLink JUCE Client.exe" "Cpp\Client\Installer\TempoLink.iss"

Write-Host "`n패키징 완료! $OutDir 폴더를 확인하세요." -ForegroundColor Cyan
