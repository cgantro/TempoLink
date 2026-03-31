Push-Location SpringBoot/ControlPlaneService
try {
  if (Test-Path .\gradlew.bat) {
    .\gradlew.bat bootRun
  } elseif (Get-Command gradle -ErrorAction SilentlyContinue) {
    gradle bootRun
  } else {
    throw "Gradle wrapper(gradlew.bat) 또는 gradle CLI가 필요합니다."
  }
} finally {
  Pop-Location
}
