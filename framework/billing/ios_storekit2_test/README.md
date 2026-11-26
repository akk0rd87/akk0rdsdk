# Биллинг iOS на StoreKit 2 — черновик

`../ios_storekit2/` — реализация `iOSBillingManager` на StoreKit 2, замена `core/platformwrapper/ios/ios_billing.mm`
(StoreKit 1). Написана 2026-11-27 на Windows, **на Mac ни разу не собиралась и не запускалась**. Основа — биллинг JCross
(`proj.ios/JCross/services/billing/BillingService.swift` и его тесты), обобщённый для SDK.

## Файлы

- `../ios_storekit2/StoreKit2Billing.swift` — логика на StoreKit 2 и C-функции `akk_sk2_*` (`@_cdecl`).
- `../ios_storekit2/storekit2_bridge.h` — их прототипы для C++ (типы указателей должны совпадать со Swift вручную).
- `../ios_storekit2/ios_billing_storekit2.cpp` — `iOSBillingManager` через них, события — в `BillingCallbackObserver`.
- `StoreKit2BillingTests.swift` + `AkkordBillingTest.storekit` — XCTest на тестовом App Store (`SKTestSession`, iOS 17+
  на симуляторе), реализация компилируется прямо в тестовый бандл.

Приложение подключает **либо** папку `ios_storekit2` (оба файла), **либо** `ios_billing.mm` — иначе два определения
`iOSBillingManager`. Шаблон `AkkordSDLApp` (`tools/ios/akkord_sdl.yml`) сейчас включает `ios_billing.mm` всегда.

## Что проверить на Mac

1. Собирается: Swift (`@_cdecl` с параметрами `@convention(c)`, глобальная `@MainActor var`, `product.price * 1_000_000`
   для `Decimal`, `priceFormatStyle.currencyCode`) и C++ (`ios_billing_storekit2.cpp` с заголовками SDK).
2. Тесты: таргет `CiBillingTests` в `ci/proj.ios/project.yml` —
   `xcodebuild test -project CiTest.xcodeproj -scheme CiBillingTests -destination 'platform=iOS Simulator,name=<iPhone с iOS 17+>'`.
   Сомнительные места: тип `SKTestTransaction.identifier` и совпадение его строки с `String(transaction.id)`;
   приходит ли возврат денег (`refundTransaction`) в `Transaction.updates` (тест `testARefundWhileRunningIsReportedAsRevoked`);
   повторная доставка незакрытого потребляемого товара.
3. Подключение в игре на SDL (Pixel Tanks: `apps/ios/project.yml` — вместо `ios_billing.mm` две строки
   `ios_storekit2`, `SWIFT_VERSION` у таргета), покупка в sandbox на устройстве.
4. Job в `.github/workflows/actions.yml` (macOS): сборка `CiBillingTests` и `xcodebuild test`.

## Поведение (решено при переносе из JCross)

- Запуск: `Transaction.updates` на всё время работы + тихий `currentEntitlements` → каждый непотребляемый товар
  `Restored`, в конце `PurchasesQueryFinishedCallback(true)`. `currentEntitlements` не бросает ошибок: без сети и на
  новом устройстве до синхронизации он может быть пуст, поэтому «нет товара» само по себе — не отзыв (игре — правило
  двух успешных запросов подряд).
- Возврат денег, пока игра работает, — `PurchaseRevokedCallback(sku)` сразу (`revocationDate` в `Transaction.updates`).
- Потребляемый товар закрывается (`finish`) в `ConsumeProductItem` после выдачи, непотребляемый — сразу.
- Кнопка «Восстановить» — `AppStore.sync()` (может попросить войти в App Store) и снова `currentEntitlements`.
- `QueryProductDetails` → `ProductDetailsCallback`: `displayPrice`, цена в миллионных, валюта, `displayName`.
- Неподтверждённая подписью App Store покупка не выдаётся и не закрывается. Подписки пока не продаются: закрываются
  без выдачи.
