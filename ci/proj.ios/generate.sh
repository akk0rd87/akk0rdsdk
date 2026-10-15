#!/bin/bash
# Генерирует CiTest.xcodeproj (XcodeGen) и ставит pod'ы. Минимальная версия iOS — ios_deployment_target из
# tools/ios/pods.yml, как у Podfile: так проект и pod'ы не расходятся.
#
#   AKKORD_SDK_HOME=<корень SDK> ./generate.sh
set -euo pipefail

: "${AKKORD_SDK_HOME:?AKKORD_SDK_HOME is not set}"
cd "$(dirname "$0")"
AKKORD_IOS_DEPLOYMENT_TARGET=$(ruby -ryaml -e 'puts YAML.load_file(ARGV[0]).fetch("ios_deployment_target")' \
    "$AKKORD_SDK_HOME/tools/ios/pods.yml")
export AKKORD_IOS_DEPLOYMENT_TARGET
echo "iOS deployment target: $AKKORD_IOS_DEPLOYMENT_TARGET"
xcodegen generate
pod install
