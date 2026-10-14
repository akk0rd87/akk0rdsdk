#!/bin/bash

# Переменные AKKORSDK_POD_* для Podfile, которые читают версии из окружения. Сами версии — в pods.yml рядом
# (единственный источник); новые Podfile читают pods.yml напрямую. Запускать через source, нужен AKKORD_SDK_HOME.

AKKORSDK_PODS_YML="$AKKORD_SDK_HOME/tools/ios/pods.yml"
akkorsdk_pod_value() {
    ruby -ryaml -e 'value = ARGV[1].split("/").reduce(YAML.load_file(ARGV[0])) { |node, key| node.fetch(key) }; puts value' "$AKKORSDK_PODS_YML" "$1"
}

export AKKORSDK_POD_TARGET=$(akkorsdk_pod_value ios_deployment_target)

export AKKORSDK_POD_GOOGLE_MOBILE_ADS_SDK_VERSION=$(akkorsdk_pod_value pods/Google-Mobile-Ads-SDK)
export AKKORSDK_POD_YANDEX_MOBILE_ADS_SDK_VERSION=$(akkorsdk_pod_value pods/YandexMobileAds)
