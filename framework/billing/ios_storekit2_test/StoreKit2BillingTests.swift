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

    private var session: SKTestSession!
    private var events: [String] = []
    private var billing: StoreKit2Billing!

    override func setUp() async throws {
        try await super.setUp()
        guard #available(iOS 17, *) else { throw XCTSkip("SKTestSession.buyProduct(identifier:) needs iOS 17") }
        session = try SKTestSession(configurationFileNamed: "AkkordBillingTest")
        session.resetToDefaultState()
        session.clearTransactions()
        session.disableDialogs = true
        events = []
        billing = StoreKit2Billing(events: StoreKit2Events(
            purchase: { [unowned self] _, sku, action in
                self.events.append((action == StoreKit2Billing.bought ? "bought " : "restored ") + sku)
            },
            consumed: { [unowned self] _, sku in self.events.append("consumed " + sku) },
            queryFinished: { [unowned self] success in self.events.append("query finished \(success)") },
            revoked: { [unowned self] sku in self.events.append("revoked " + sku) },
            products: { [unowned self] list in
                self.events.append("products " + list.map { "\($0.sku) \($0.priceMicros) \($0.currency)" }.joined(separator: ", "))
            }))
    }

    override func tearDown() async throws {
        billing?.stop()
        session?.clearTransactions()
        try await super.tearDown()
    }

    // ---- запуск ----

    func testLaunchWithoutPurchasesOnlyFinishesTheQuery() async {
        await billing.reportEntitlements()
        XCTAssertEqual(events, ["query finished true"])
    }

    /// Купленное восстанавливается при запуске без окна входа - как после переустановки или на новом устройстве
    func testLaunchRestoresANonConsumableBoughtElsewhere() async throws {
        try await buyOutsideTheBilling(nonConsumable)
        await billing.reportEntitlements()
        XCTAssertEqual(events, ["restored \(nonConsumable)", "query finished true"])
    }

    /// Потребляемый товар после выдачи не возвращается ни при запуске, ни кнопкой
    func testAConsumedProductIsNotRestored() async throws {
        await billing.purchase(consumable)
        let token = try XCTUnwrap(lastToken(of: consumable))
        await billing.consume(token)
        events = []

        await billing.restorePurchases()

        XCTAssertEqual(events, ["query finished true"])
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
        XCTAssertEqual(events, ["restored \(nonConsumable)", "query finished true"])
    }

    // ---- возврат денег ----

    func testARefundedPurchaseIsNotRestored() async throws {
        try await buyOutsideTheBilling(nonConsumable)
        let transaction = try XCTUnwrap(session.allTransactions().first { $0.productIdentifier == nonConsumable })
        try session.refundTransaction(identifier: transaction.identifier)

        await billing.reportEntitlements()

        XCTAssertEqual(events, ["query finished true"])
    }

    /// Возврат денег, пока игра работает, приходит через Transaction.updates сразу
    func testARefundWhileRunningIsReportedAsRevoked() async throws {
        billing.start()
        await billing.purchase(nonConsumable)
        let transaction = try XCTUnwrap(session.allTransactions().first { $0.productIdentifier == nonConsumable })

        try session.refundTransaction(identifier: transaction.identifier)

        try await waitFor("revoked \(nonConsumable)")
    }

    // ---- Ask to Buy ----

    func testAnAskToBuyPurchaseArrivesAfterApproval() async throws {
        session.askToBuyEnabled = true
        billing.start()

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
    private func buyOutsideTheBilling(_ sku: String) async throws {
        guard #available(iOS 17, *) else { throw XCTSkip("needs iOS 17") }
        let transaction = try await session.buyProduct(identifier: sku)
        await transaction.finish()
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

    private func waitFor(_ event: String, seconds: TimeInterval = 10) async throws {
        let deadline = Date().addingTimeInterval(seconds)
        while !events.contains(event), Date() < deadline {
            try await Task.sleep(nanoseconds: 100_000_000)
        }
        XCTAssertTrue(events.contains(event), "\(event) did not come; events: \(events)")
    }
}
