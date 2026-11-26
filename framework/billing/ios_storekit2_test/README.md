# Биллинг iOS на StoreKit 2

`../ios_storekit2/` — реализация `iOSBillingManager` на StoreKit 2, замена `core/platformwrapper/ios/ios_billing.mm`
(StoreKit 1). Написана 2026-11-27 на Windows, **проверена на Mac в тот же день**: собирается, тесты проходят, игра на
ней собрана и запущена на устройстве (`ios_storekit2` в SDK, `ios-storekit2` в Pixel Tanks). Основа — биллинг JCross
(`proj.ios/JCross/services/billing/BillingService.swift` и его тесты), обобщённый для SDK.

## Файлы

- `../ios_storekit2/StoreKit2Billing.swift` — логика на StoreKit 2 и C-функции `akk_sk2_*` (`@_cdecl`).
- `../ios_storekit2/storekit2_bridge.h` — их прототипы для C++ (типы указателей должны совпадать со Swift вручную).
- `../ios_storekit2/ios_billing_storekit2.cpp` — `iOSBillingManager` через них, события — в `BillingCallbackObserver`.
- `StoreKit2BillingTests.swift` + `AkkordBillingTest.storekit` — XCTest на тестовом App Store (`SKTestSession`, iOS 17+
  на симуляторе), реализация компилируется прямо в тестовый бандл.
- `../../../ci/proj.ios/pick_simulator.py` — выбор симулятора для этих тестов (iOS 17+), им же пользуется CI.

Приложение подключает **либо** папку `ios_storekit2` (оба файла), **либо** `ios_billing.mm` — иначе два определения
`iOSBillingManager`. Шаблон `AkkordSDLApp` (`tools/ios/akkord_sdl.yml`) включает `ios_billing.mm` всегда, поэтому
приложение на шаблоне пока остаётся на StoreKit 1; игры, которые перечисляют исходники SDK сами (Pixel Tanks), просто
меняют одну строку на две.

## Как проверять

```
export AKKORD_SDK_HOME=<корень SDK>
cd ci/proj.ios && ./generate.sh
udid=$(python3 pick_simulator.py)
xcodebuild test -project CiTest.xcodeproj -scheme CiBillingTests -destination "id=$udid" CODE_SIGNING_ALLOWED=NO
```

Мост в C++ собирается в том же тестовом бандле (`ios_billing_storekit2.cpp` в таргете `CiBillingTests`): так
проверяется и компиляция с заголовками SDK, и связь с теми самыми функциями Swift. В `CiTestSDL` его не подключить —
там шаблон `AkkordSDLApp` с `ios_billing.mm`, и `iOSBillingManager` оказался бы определён дважды.

## Что выяснилось на Mac (2026-11-27)

- **`@_cdecl` и внутренние типы.** Публичная функция не принимает параметры внутренних типов: `typealias AkkSk2*Fn`
  пришлось сделать `public`. Единственная ошибка компиляции во всём переносе.
- **`SKTestTransaction.identifier` — это `UInt`**, и его строка совпадает с `String(transaction.id)` (`UInt64`):
  токен покупки, который реализация отдаёт игре, им же и возвращается в `session.refundTransaction(identifier:)`.
- **Возврат денег приходит в `Transaction.updates`** с заполненным `revocationDate` — через секунду после
  `refundTransaction`. То есть `PurchaseRevokedCallback` работает, и на тестовом магазине это проверяемо.
- **Одна покупка приходит дважды**: ответом окна покупки и через `Transaction.updates`. Для непотребляемого товара
  это безвредно (игра снова узнаёт, что он куплен), а потребляемый был бы выдан два раза — поэтому пока покупка не
  выдана и не закрыта (лежит в `unfinishedConsumables`), второй раз о ней не сообщается. Это **отличие от
  первоначального замысла**, вписанное по итогам прогона.
- **Тестовый магазин один на процесс.** Сначала каждый тест создавал свой `SKTestSession` и свой `StoreKit2Billing`,
  а XCTest держит объекты тестов до конца набора: несколько живых слушателей `Transaction.updates` работали
  одновременно, и отзыв покупки доставался не тому — тест `testARefundWhileRunningIsReportedAsRevoked` падал в наборе
  через прогон, проходя в одиночку. Теперь сессия и биллинг — одни на набор, как и в приложении (`akkSk2Billing`
  единственный), а события общий биллинг сыпет в тот тест, который идёт.
- **Возврат денег — за ту покупку, о которой сказала реализация.** `session.allTransactions()` помнит и покупки
  соседних тестов того же товара (`clearTransactions` убирает их не сразу), и тест возвращал деньги за чужую, старую:
  отзыва по ней StoreKit уже не присылает. Теперь тест берёт токен из события `purchase`.
- **Лог.** Реализация пишет, что вернул запрос товаров (`[Billing] products: ...`), что нашлось среди действующих
  покупок (`[Billing] entitlements: ...`) и о каждом непотребляемом товаре. Без этого на устройстве дорога молчит, и
  «покупка не вернулась» не отличить от «запрос не дошёл».

## Проверено на устройстве (iPad, Pixel Tanks, sandbox)

- Игра собирается и запускается с этой реализацией; `BillingManager::Init` отвечает успехом («Billing: started»).
- Тихий запрос действующих покупок проходит: `[Billing] entitlements: none` на чистом устройстве.
- **Цены приходят из настоящего App Store**: `[Billing] products: tanks_paints_all 599.00 RUB, ... ,
  tanks_turn_off_ads 449.00 RUB` — семь товаров, то есть `Product.products(for:)`, `displayPrice`, цена в миллионных
  и валюта работают на живом магазине.

Что осталось за живой рукой (нужен вход в sandbox-аккаунт на устройстве, окно App Store тестом не нажать): сама
покупка «без рекламы», возвращение покупки после переустановки, кнопка RESTORE PURCHASES. Возврат денег в sandbox
Apple не предоставляет вовсе — он проверен на тестовом магазине (`SKTestSession`), см. выше.

## Поведение (решено при переносе из JCross)

- Запуск: `Transaction.updates` на всё время работы + тихий `currentEntitlements` → каждый непотребляемый товар
  `Restored`, в конце `PurchasesQueryFinishedCallback(true)`. `currentEntitlements` не бросает ошибок: без сети и на
  новом устройстве до синхронизации он может быть пуст, поэтому «нет товара» само по себе — не отзыв (игре — правило
  двух успешных запросов подряд).
- Возврат денег, пока игра работает, — `PurchaseRevokedCallback(sku)` сразу (`revocationDate` в `Transaction.updates`).
- Потребляемый товар закрывается (`finish`) в `ConsumeProductItem` после выдачи, непотребляемый — сразу; повторная
  доставка той же покупки в этом же запуске потребляемый товар второй раз не выдаёт (см. выше).
- Кнопка «Восстановить» — `AppStore.sync()` (может попросить войти в App Store) и снова `currentEntitlements`.
- `QueryProductDetails` → `ProductDetailsCallback`: `displayPrice`, цена в миллионных, валюта, `displayName`.
- Неподтверждённая подписью App Store покупка не выдаётся и не закрывается. Подписки пока не продаются: закрываются
  без выдачи.
