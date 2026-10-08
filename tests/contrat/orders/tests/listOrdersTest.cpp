#include <gtest/gtest.h>

#include <QList>

#include "ContractTestSupport.h"
#include "orders/model/Order.h"
#include "orders/network/RestApiClient.h"

using namespace ecommerce::orders;
using ecommerce::orders::test::mockBaseUrl;
using ecommerce::orders::test::waitUntil;

// Vérifie que GET /orders du mock renvoie la liste d'exemples du contrat et que
// RestApiClient la désérialise correctement.
TEST(ListOrdersContract, ReturnsOrdersFromMock) {
    RestApiClient client(mockBaseUrl());

    bool done = false;
    QList<Order> received;
    ApiError error;
    client.listOrders([&](const QList<Order>& orders, const ApiError& err) {
        received = orders;
        error = err;
        done = true;
    });

    ASSERT_TRUE(waitUntil(done)) << "pas de réponse du mock (MOCK_BASE_URL="
                                 << mockBaseUrl().toStdString() << ")";
    ASSERT_FALSE(error.failed) << error.message.toStdString();
    ASSERT_FALSE(received.isEmpty());
    EXPECT_EQ(received.first().id().toStdString(), "ORD-1001");
    EXPECT_EQ(received.first().status().toStdString(), "CONFIRMED");
}
