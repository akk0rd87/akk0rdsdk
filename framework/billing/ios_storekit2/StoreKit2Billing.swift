// Биллинг iOS на StoreKit 2 - замена core/platformwrapper/ios/ios_billing.mm (StoreKit 1). Приложение подключает ЛИБО
// эту папку (StoreKit2Billing.swift + ios_billing_storekit2.cpp), ЛИБО ios_billing.mm: обе реализуют iOSBillingManager.
// Нужен iOS 15 (tools/ios/pods.yml: ios_deployment_target 15.0).
//
// ЧЕРНОВИК (2026-11-27, написан на Windows без компилятора): на Mac ещё не собирался и не запускался - см.
// ../ios_storekit2_test/README.md, что проверить.
//
// Отличия от StoreKit 1:
// - при запуске покупки восстанавливаются ТИХО: Transaction.currentEntitlements отдаёт действующие покупки без окна
//   входа в App Store (и после переустановки, и на новом устройстве); кнопка «Восстановить покупки» (RestorePurchases)
//   нужна, только чтобы попросить App Store синхронизироваться (AppStore.sync) - правило Apple, кнопка должна быть;
// - Transaction.updates всё время работы приносит покупки, завершившиеся вне окна покупки (Ask to Buy, отложенная
//   оплата, промо-покупка из App Store), незакрытые покупки прошлого запуска и ОТЗЫВЫ (возврат денег) - о них игра
//   узнаёт сразу (PurchaseRevokedCallback);
// - потребляемый товар закрывается (finish) только в ConsumeProductItem, после выдачи: упала игра до выдачи - покупка
//   придёт снова при следующем запуске; непотребляемый - сразу.
//
// Связь с C++ - через C-функции (@_cdecl сюда, указатели на функции обратно), а не через заголовок <Модуль>-Swift.h:
// его имя у каждого приложения своё. Прототипы - storekit2_bridge.h.

import Foundation
import StoreKit

// Куда реализация сообщает о покупках: в приложении - в C++ (BillingCallbackObserver), в тестах - в список событий
struct StoreKit2Events {
    // action: 0 - восстановлена (BillingManager::OperAction::Restored), 1 - куплена (Bought)
    var purchase: (_ token: String, _ sku: String, _ action: Int32) -> Void
    var consumed: (_ token: String, _ sku: String) -> Void
    var queryFinished: (_ success: Bool) -> Void
    var revoked: (_ sku: String) -> Void
    var products: (_ products: [StoreKit2Product]) -> Void
}

struct StoreKit2Product: Equatable {
    let sku: String
    let priceLabel: String   // displayPrice: цена с символом валюты, как её показывает App Store
    let priceMicros: Int64   // цена в миллионных долях валюты
    let currency: String     // ISO 4217
    let title: String        // displayName
}

@MainActor
final class StoreKit2Billing {
    static let restored: Int32 = 0
    static let bought: Int32 = 1

    private let events: StoreKit2Events
    private var updatesTask: Task<Void, Never>?
    private var products: [String: Product] = [:]              // из QueryProductDetails и покупок: не спрашивать снова
    private var unfinishedConsumables: [String: Transaction] = [:]  // токен -> выданная игре, ещё не закрытая покупка

    init(events: StoreKit2Events) {
        self.events = events
    }

    // Init: слушатель Transaction.updates на всё время работы и тихая проверка действующих покупок
    func start() {
        guard updatesTask == nil else { return }
        updatesTask = Task { [weak self] in
            for await result in Transaction.updates {
                await self?.handle(result, action: Self.bought)
            }
        }
        Task { await reportEntitlements() }
    }

    func stop() {
        updatesTask?.cancel()
        updatesTask = nil
    }

    // Действующие покупки -> «восстановлена» каждая, в конце - «запрос завершён».
    // currentEntitlements не бросает ошибок: без сети он отдаёт то, что StoreKit помнит, а на новом устройстве до
    // первой синхронизации может быть пуст. Поэтому success здесь всегда true, и игра, отзывая товар по его
    // отсутствию, должна требовать подтверждения (два успешных запроса подряд); явный отзыв приходит в revoked.
    func reportEntitlements() async {
        var reported: [String] = []
        for await result in Transaction.currentEntitlements {
            guard case .verified(let transaction) = result, transaction.revocationDate == nil else { continue }
            // Потребляемых здесь нет (StoreKit их не держит); подписки SDK пока не продаёт
            guard transaction.productType == .nonConsumable else { continue }
            reported.append(transaction.productID)
            events.purchase(String(transaction.id), transaction.productID, Self.restored)
        }
        // В лог: иначе на устройстве эта дорога молчит, и «покупка не вернулась» не отличить от «запрос не дошёл»
        NSLog("[Billing] entitlements: \(reported.isEmpty ? "none" : reported.joined(separator: ", "))")
        events.queryFinished(true)
    }

    // Кнопка «Восстановить покупки»: AppStore.sync может попросить войти в App Store - только по действию игрока
    func restorePurchases() async {
        do {
            try await AppStore.sync()
        } catch {
            NSLog("[Billing] AppStore.sync failed: \(error)")
        }
        await reportEntitlements()
    }

    func queryProducts(_ skus: [String]) async {
        do {
            let found = try await Product.products(for: skus)
            var out: [StoreKit2Product] = []
            for product in found {
                products[product.id] = product
                out.append(StoreKit2Product(
                    sku: product.id,
                    priceLabel: product.displayPrice,
                    priceMicros: NSDecimalNumber(decimal: product.price * 1_000_000).int64Value,
                    currency: product.priceFormatStyle.currencyCode,
                    title: product.displayName))
            }
            NSLog("[Billing] products: \(out.isEmpty ? "none" : out.map { "\($0.sku) \($0.priceLabel)" }.joined(separator: ", "))")
            events.products(out)
        } catch {
            NSLog("[Billing] Product.products failed: \(error)")
            events.products([])
        }
    }

    func purchase(_ sku: String) async {
        do {
            var product = products[sku]
            if product == nil {
                product = try await Product.products(for: [sku]).first
                products[sku] = product
            }
            guard let product else {
                NSLog("[Billing] \(sku) not found in App Store")
                return
            }
            switch try await product.purchase() {
            case .success(let verification):
                await handle(verification, action: Self.bought)
            case .pending:
                // Ask to Buy, SCA: когда завершится, придёт через Transaction.updates
                NSLog("[Billing] \(sku) purchase is pending")
            case .userCancelled:
                break
            @unknown default:
                break
            }
        } catch {
            NSLog("[Billing] purchase of \(sku) failed: \(error)")
        }
    }

    // Игра выдала потребляемый товар: закрываем покупку
    func consume(_ token: String) async {
        if let transaction = unfinishedConsumables.removeValue(forKey: token) {
            await transaction.finish()
            events.consumed(token, transaction.productID)
            return
        }
        // Выдана в прошлом запуске, а Transaction.updates ещё не принёс её в этом
        for await result in Transaction.unfinished {
            if case .verified(let transaction) = result, String(transaction.id) == token {
                await transaction.finish()
                events.consumed(token, transaction.productID)
                return
            }
        }
        NSLog("[Billing] consume: no unfinished purchase \(token)")
    }

    // Покупка из окна покупки или из Transaction.updates
    func handle(_ result: VerificationResult<Transaction>, action: Int32) async {
        guard case .verified(let transaction) = result else {
            // Подпись App Store не сошлась: товар не выдаём и покупку не закрываем
            NSLog("[Billing] transaction failed verification")
            return
        }
        let token = String(transaction.id)
        if transaction.revocationDate != nil {
            events.revoked(transaction.productID)
            await transaction.finish()
            return
        }
        switch transaction.productType {
        case .consumable:
            // StoreKit приносит одну и ту же покупку дважды - ответом окна покупки и через Transaction.updates
            // (проверено на симуляторе 2026-11-27). Потребляемый товар игра выдала бы два раза, поэтому пока
            // покупка не выдана и не закрыта (лежит здесь), второй раз о ней не говорим. После выдачи токен
            // уходит, и повторная доставка в НОВОМ запуске (игра упала до выдачи) снова выдаст товар - так и
            // задумано. Непотребляемый повторяется безвредно: игра просто снова узнаёт, что он куплен.
            if unfinishedConsumables[token] != nil { return }
            // Закроется в consume, когда игра выдаст товар
            unfinishedConsumables[token] = transaction
            events.purchase(token, transaction.productID, action)
        case .nonConsumable:
            NSLog("[Billing] \(transaction.productID): \(action == Self.bought ? "bought" : "restored")")
            events.purchase(token, transaction.productID, action)
            await transaction.finish()
        default:
            // Подписок SDK пока не продаёт: закрываем, чтобы StoreKit не присылал их снова
            NSLog("[Billing] \(transaction.productID): product type \(transaction.productType) is not handled")
            await transaction.finish()
        }
    }
}

// ---- C-интерфейс для ios_billing_storekit2.cpp (storekit2_bridge.h) ----

// Публичные: типы параметров публичной функции (@_cdecl ниже) не могут быть внутренними
public typealias AkkSk2PurchaseFn = @convention(c) (UnsafePointer<CChar>, UnsafePointer<CChar>, Int32) -> Void
public typealias AkkSk2ConsumedFn = @convention(c) (UnsafePointer<CChar>, UnsafePointer<CChar>) -> Void
public typealias AkkSk2QueryFinishedFn = @convention(c) (Int32) -> Void
public typealias AkkSk2RevokedFn = @convention(c) (UnsafePointer<CChar>) -> Void
public typealias AkkSk2ProductFn = @convention(c) (UnsafePointer<CChar>, UnsafePointer<CChar>, Int64, UnsafePointer<CChar>, UnsafePointer<CChar>) -> Void
public typealias AkkSk2ProductsDoneFn = @convention(c) () -> Void

@MainActor private var akkSk2Billing: StoreKit2Billing?

// Вызовы приходят из потока игры (на iOS SDL работает на главном потоке, но не полагаемся): строки копируются сразу,
// работа идёт на MainActor. Обратные вызовы - тоже с главного потока, как было у StoreKit 1.
@_cdecl("akk_sk2_start")
public func akkSk2Start(_ onPurchase: AkkSk2PurchaseFn, _ onConsumed: AkkSk2ConsumedFn,
                        _ onQueryFinished: AkkSk2QueryFinishedFn, _ onRevoked: AkkSk2RevokedFn,
                        _ onProduct: AkkSk2ProductFn, _ onProductsDone: AkkSk2ProductsDoneFn) {
    let events = StoreKit2Events(
        purchase: { token, sku, action in token.withCString { t in sku.withCString { s in onPurchase(t, s, action) } } },
        consumed: { token, sku in token.withCString { t in sku.withCString { s in onConsumed(t, s) } } },
        queryFinished: { success in onQueryFinished(success ? 1 : 0) },
        revoked: { sku in sku.withCString { onRevoked($0) } },
        products: { list in
            for p in list {
                p.sku.withCString { sku in p.priceLabel.withCString { label in p.currency.withCString { currency in
                    p.title.withCString { title in onProduct(sku, label, p.priceMicros, currency, title) } } } }
            }
            onProductsDone()
        })
    Task { @MainActor in
        guard akkSk2Billing == nil else { return }
        let billing = StoreKit2Billing(events: events)
        akkSk2Billing = billing
        billing.start()
    }
}

@_cdecl("akk_sk2_query_products")
public func akkSk2QueryProducts(_ skus: UnsafePointer<UnsafePointer<CChar>?>?, _ count: Int32) {
    var list: [String] = []
    if let skus {
        for i in 0..<Int(count) {
            if let sku = skus[i] { list.append(String(cString: sku)) }
        }
    }
    Task { @MainActor in await akkSk2Billing?.queryProducts(list) }
}

@_cdecl("akk_sk2_restore")
public func akkSk2Restore() {
    Task { @MainActor in await akkSk2Billing?.restorePurchases() }
}

@_cdecl("akk_sk2_purchase")
public func akkSk2Purchase(_ sku: UnsafePointer<CChar>) {
    let product = String(cString: sku)
    Task { @MainActor in await akkSk2Billing?.purchase(product) }
}

@_cdecl("akk_sk2_consume")
public func akkSk2Consume(_ token: UnsafePointer<CChar>) {
    let purchase = String(cString: token)
    Task { @MainActor in await akkSk2Billing?.consume(purchase) }
}
