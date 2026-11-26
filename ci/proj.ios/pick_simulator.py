#!/usr/bin/env python3
"""Печатает UDID симулятора для тестов биллинга: самый новый iPhone с iOS 17 или новее.

Зачем именно 17+: `SKTestSession.buyProduct(identifier:)` появился в iOS 17, и на более старой тесты биллинга
молча пропускаются (`XCTSkip` в `StoreKit2BillingTests`) — шаг CI при этом зеленеет, ничего не проверив. Поэтому
без подходящего симулятора скрипт заканчивается ошибкой, а не выбирает первый попавшийся.

    udid=$(python3 pick_simulator.py) || exit 1
    xcodebuild test -project CiTest.xcodeproj -scheme CiBillingTests -destination "id=$udid"

Отдельным файлом, а не строкой в actions.yml: внутри блока YAML у кода появляются отступы, и `python3 -c` падает
на них (IndentationError). Заодно его можно прогнать на своей машине — ровно то, что делает CI.
"""

from __future__ import annotations

import json
import subprocess
import sys


def main() -> int:
    listing = subprocess.run(["xcrun", "simctl", "list", "devices", "available", "--json"],
                             capture_output=True, text=True, check=True)
    best: tuple[tuple[int, ...], str, str] | None = None
    for runtime, devices in json.loads(listing.stdout)["devices"].items():
        if "iOS" not in runtime:
            continue
        # «com.apple.CoreSimulator.SimRuntime.iOS-17-5» → (17, 5)
        try:
            version = tuple(int(part) for part in runtime.rsplit(".", 1)[-1].replace("iOS-", "").split("-"))
        except ValueError:
            continue
        if version < (17,):
            continue
        for device in devices:
            if device.get("isAvailable") and device.get("name", "").startswith("iPhone"):
                if best is None or version > best[0]:
                    best = (version, device["udid"], device["name"])
    if best is None:
        print("нет симулятора iPhone с iOS 17 или новее: тесты биллинга без него только пропускаются",
              file=sys.stderr)
        return 1
    version, udid, name = best
    print(f"{name} iOS {'.'.join(str(part) for part in version)}", file=sys.stderr)
    print(udid)
    return 0


if __name__ == "__main__":
    sys.exit(main())
