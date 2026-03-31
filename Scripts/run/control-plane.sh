#!/usr/bin/env bash
set -euo pipefail

cd SpringBoot/ControlPlaneService
if [[ -x "./gradlew" ]]; then
  ./gradlew bootRun
elif command -v gradle >/dev/null 2>&1; then
  gradle bootRun
else
  echo "Gradle wrapper(./gradlew) or gradle CLI is required." >&2
  exit 1
fi
