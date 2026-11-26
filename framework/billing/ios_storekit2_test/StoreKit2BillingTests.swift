import StoreKit
import StoreKitTest
import XCTest

// Биллинг на StoreKit 2 (../ios_storekit2/StoreKit2Billing.swift) на тестовом App Store (SKTestSession с
// AkkordBillingTest.storekit): что реализация сообщает игре при запуске, покупке, выдаче, кнопке «Восстановить», возврате
// денег, Ask to Buy и запросе товаров. Порт тестов BillingService из JCross (proj.ios/JCrossTests/billing), без
// товаров JCross. Файл реализации компилируется прямо в этот тестовый бандл (без @testable import).
// ЧЕРНОВИК: на Mac ещё не запускался - см. README.md рядом.
@MainActor
final class StoreKit2BillingTests: XCTestCase {
    private let nonConsumable = "akk_test_non_consumable"
    private let consumable = "akk_test_consumable"

    // Одна сессия тестового магазина и один биллинг на весь набор, а не свои у каждого теста - так же, как в
    // приложении, где биллинг единственный (`akkSk2Billing`). XCTest держит объекты тестов до конца набора,
    // поэтому «свой у каждого» означало несколько живых слушателей Transaction.updates и несколько сессий
    // тестового магазина сразу. Отзыв покупки StoreKit отдаёт одному слушателю, и доставался он чужому:
    // `testARefundWhileRunningIsReportedAsRevoked` падал в наборе через прогон, проходя в одиночку (измерено
    // 2026-11-27: прогон 1 - падение, прогон 2 - чисто). Состояние магазина чистится перед каждым тестом, а
    // события общий биллинг сыпет в тот тест, который идёт (`current`).
    private static var sharedSession: SKTestSession?
    private static var sharedBilling: StoreKit2Billing?
    private static weak var current: StoreKit2BillingTests?
    private var session: SKTestSession!
    private var events: [String] = []
    private var billing: StoreKit2Billing!
    // Токен последней покупки, о которой сказала реализация. Нужен, чтобы возвращать деньги именно за неё:
    // `session.allTransactions()` помнит и покупки соседних тестов того же товара (`clearTransactions` их не
    // убирает сразу), и тест возвращал деньги за чужую, старую, - отзыва по ней StoreKit уже не присылает.
    private var lastToken: String?

    override func setUp() async throws {
        try await super.setUp()
        guard #available(iOS 17, *) else { throw XCTSkip("SKTestSession.buyProduct(identifier:) needs iOS 17") }
        if Self.sharedSession == nil {
            Self.sharedSession = try SKTestSession(configurationFileNamed: "AkkordBillingTest")
        }
        session = Self.sharedSession
        session.resetToDefaultState()
        session.clearTransactions()
        session.disableDialogs = true
        lastToken = nil
        Self.current = self
        events = []
        if Self.sharedBilling == nil {
            Self.sharedBilling = StoreKit2Billing(events: StoreKit2Events(
                purchase: { token, sku, action in
                    Self.current?.lastToken = token
                    Self.current?.events.append((action == StoreKit2Billing.bought ? "bought " : "restored ") + sku)
                },
                consumed: { _, sku in Self.current?.events.append("consumed " + sku) },
                queryFinished: { success in Self.current?.events.append("query finished \(success)") },
                revoked: { sku in Self.current?.events.append("revoked " + sku) },
                products: { list in
                    Self.current?.events.append("products " + list.map { "\($0.sku) \($0.priceMicros) \($0.currency)" }.joined(separator: ", "))
                }))
            // Слушатель Transaction.updates поднимается один раз на набор: в приложении он тоже один и живёт
            // всё время работы. Вместе со слушателем start() сообщает действующие покупки - дождёмся этого
            // ответа и начнём первый тест с чистого списка, иначе «query finished true» попадал в его события.
            Self.sharedBilling?.start()
            let deadline = Date().addingTimeInterval(10)
            while !events.contains("query finished true"), Date() < deadline {
                try await Task.sleep(nanoseconds: 50_000_000)
            }
            events = []
        }
        billing = Self.sharedBilling
    }

    override func tearDown() async throws {
        // Ни биллинг, ни сессию здесь не закрываем: они общие на набор (см. выше). Останавливать слушателя
        // между тестами как раз и нельзя - из-за этого отзыв покупки уходил не туда.
        billing = nil
        session?.clearTransactions()
        session = nil
        try await super.tearDown()
    }

    // ---- запуск ----

    /// Проверяет, чем кончился список событий, а не весь список целиком.
    ///
    /// Слушатель `Transaction.updates` живёт всё время набора, как и в приложении, поэтому покупка, сделанная
    /// мимо реализации (`buyOutsideTheBilling` - это «купили на другом устройстве»), приходит ещё и сама, как
    /// «bought». Так StoreKit 2 и задуман, и момент этой доставки не наш: сравнение со всем списком делало тесты
    /// шаткими (измерено 2026-11-27). Смысл проверки от этого не страдает - важно, что ответил запрос покупок.
    private func assertTail(_ expected: [String], _ message: String = "") {
        XCTAssertEqual(Array(events.suffix(expected.count)), expected, message.isEmpty ? "события: \(events)" : message)
    }


    func testLaunchWithoutPurchasesOnlyFinishesTheQuery() async {
        await billing.reportEntitlements()
        XCTAssertEqual(events, ["query finished true"])
    }

    /// Купленное восстанавливается при запуске без окна входа - как после переустановки или на новом устройстве
    func testLaunchRestoresANonConsumableBoughtElsewhere() async throws {
        try await buyOutsideTheBilling(nonConsumable)
        await billing.reportEntitlements()
        assertTail(["restored \(nonConsumable)", "query finished true"])
    }

    /// Потребляемый товар после выдачи не возвращается ни при запуске, ни кнопкой
    func testAConsumedProductIsNotRestored() async throws {
        await billing.purchase(consumable)
        let token = try XCTUnwrap(lastToken(of: consumable))
        await billing.consume(token)
        events = []

        await billing.restorePurchases()

        assertTail(["query finished true"])
        XCTAssertFalse(events.contains("restored \(consumable)"),
                       "выданный потребляемый товар восстановился: \(events)")
    }

    // ---- покупка ----

    func testANonConsumablePurchaseIsReportedAsBoughtAndClosed() async throws {
        await billing.purchase(nonConsumable)
        XCTAssertEqual(events, ["bought \(nonConsumable)"])
        let unfinished = await unfinishedProducts()
        XCTAssertEqual(unfinished, [], "A non-consumable is finished at once")
    }

    /// Потребляемый товар закрывается только после выдачи: упала игра до выдачи - покупка придёт снова
    func testAConsumablePurchaseStaysOpenUntilConsumed() async throws {
        await billing.purchase(consumable)
        XCTAssertEqual(events, ["bought \(consumable)"])
        var unfinished = await unfinishedProducts()
        XCTAssertEqual(unfinished, [consumable])

        let token = try XCTUnwrap(lastToken(of: consumable))
        await billing.consume(token)

        XCTAssertEqual(events, ["bought \(consumable)", "consumed \(consumable)"])
        unfinished = await unfinishedProducts()
        XCTAssertEqual(unfinished, [])
    }

    func testAnUnknownProductBuysNothing() async {
        await billing.purchase("akk_test_unknown")
        XCTAssertEqual(events, [])
        XCTAssertEqual(session.allTransactions(), [])
    }

    func testRestoreButtonFindsAKnownPurchase() async throws {
        try await buyOutsideTheBilling(nonConsumable)
        await billing.restorePurchases()
        assertTail(["restored \(nonConsumable)", "query finished true"])
    }

    // ---- возврат денег ----

    func testARefundedPurchaseIsNotRestored() async throws {
        let bought = try await buyOutsideTheBilling(nonConsumable)
        try session.refundTransaction(identifier: try XCTUnwrap(UInt(String(bought))))

        await billing.reportEntitlements()

        assertTail(["query finished true"])
        XCTAssertFalse(events.contains("restored \(nonConsumable)"),
                       "возвращённая покупка восстановилась: \(events)")
    }

    /// Возврат денег, пока игра работает, приходит через Transaction.updates сразу
    func testARefundWhileRunningIsReportedAsRevoked() async throws {
        await billing.purchase(nonConsumable)
        let token = try XCTUnwrap(lastToken, "реализация не сказала токен покупки")

        try session.refundTransaction(identifier: try XCTUnwrap(UInt(token)))

        try await waitFor("revoked \(nonConsumable)")
    }

    // ---- Ask to Buy ----

    func testAnAskToBuyPurchaseArrivesAfterApproval() async throws {
        session.askToBuyEnabled = true

        await billing.purchase(nonConsumable)
        XCTAssertFalse(events.contains("bought \(nonConsumable)"), "A purchase waiting for approval must not be given yet")

        let pending = try XCTUnwrap(session.allTransactions().first { $0.productIdentifier == nonConsumable })
        try session.approveAskToBuyTransaction(identifier: pending.identifier)

        try await waitFor("bought \(nonConsumable)")
    }

    // ---- товары с ценами ----

    func testProductDetailsComeWithPrices() async {
        await billing.queryProducts([nonConsumable, consumable, "akk_test_unknown"])
        XCTAssertEqual(events.count, 1)
        // Порядок ответа App Store не гарантирован
        XCTAssertTrue(events[0].hasPrefix("products "))
        XCTAssertTrue(events[0].contains("\(nonConsumable) 4990000 USD"), events[0])
        XCTAssertTrue(events[0].contains("\(consumable) 990000 USD"), events[0])
        XCTAssertFalse(events[0].contains("akk_test_unknown"))
    }

    // ---- помощники ----

    /// Покупка мимо реализации (например, на другом устройстве): в StoreKit она есть, события ещё не было
    /// Отдаёт id покупки: возвращать деньги надо именно за неё (см. `lastToken`)
    @discardableResult
    private func buyOutsideTheBilling(_ sku: String) async throws -> UInt64 {
        guard #available(iOS 17, *) else { throw XCTSkip("needs iOS 17") }
        let transaction = try await session.buyProduct(identifier: sku)
        await transaction.finish()
        return transaction.id
    }

    private func lastToken(of sku: String) -> String? {
        session.allTransactions().last { $0.productIdentifier == sku }.map { String($0.identifier) }
    }

    private func unfinishedProducts() async -> [String] {
        var out: [String] = []
        for await result in Transaction.unfinished {
            if case .verified(let transaction) = result { out.append(transaction.productID) }
        }
        return out
    }

    /// Ждёт событие от реализации. Тридцать секунд, а не десять: тестовый магазин доставляет отзыв покупки
    /// через Transaction.updates не мгновенно, и под нагрузкой (весь набор, симулятор на занятой машине) десяти
    /// секунд не хватало - отзыв приходил позже (измерено 2026-11-27).
    private func waitFor(_ event: String, seconds: TimeInterval = 30) async throws {
        let deadline = Date().addingTimeInterval(seconds)
        while !events.contains(event), Date() < deadline {
            try await Task.sleep(nanoseconds: 100_000_000)
        }
        XCTAssertTrue(events.contains(event), "\(event) did not come; events: \(events)")
    }
}
