#include <gtest/gtest.h>

#include "ContractTestSupport.h"
#include "orders/model/Order.h"
#include "orders/network/RestApiClient.h"

using namespace ecommerce::orders;
using ecommerce::orders::test::mockBaseUrl;
using ecommerce::orders::test::waitUntil;

// Requête conforme au contrat : POST /orders avec customerId + amount → 201.
TEST(CreateOrderContract, ValidRequestReturns201) {
    RestApiClient client(mockBaseUrl());

    Order request;
    request.setCustomerId("CUST-42");
    request.setAmount(129.90);

    bool done = false;
    Order created;
    ApiError error;
    client.createOrder(request, [&](const Order& order, const ApiError& err) {
        created = order;
        error = err;
        done = true;
    });

    ASSERT_TRUE(waitUntil(done)) << "pas de réponse du mock";
    ASSERT_FALSE(error.failed) << error.message.toStdString();
    EXPECT_EQ(error.httpStatus, 201);
    EXPECT_EQ(created.status().toStdString(), "PENDING");
}

// Requête non conforme (customerId manquant) : le dispatcher JSON_BODY du
// contrat route vers l'exemple d'erreur → 400.
TEST(CreateOrderContract, InvalidRequestReturns400) {
    RestApiClient client(mockBaseUrl());

    Order request;  // ni customerId, ni id/status
    request.setAmount(0.0);

    bool done = false;
    ApiError error;
    client.createOrder(request, [&](const Order&, const ApiError& err) {
        error = err;
        done = true;
    });

    ASSERT_TRUE(waitUntil(done)) << "pas de réponse du mock";
    EXPECT_TRUE(error.failed);
    EXPECT_EQ(error.httpStatus, 400);
}
