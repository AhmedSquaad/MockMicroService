#include <gtest/gtest.h>

#include <QJsonObject>

#include "orders/model/Order.h"

using ecommerce::orders::Order;

TEST(OrderTest, DeserializesFromContractJson) {
    QJsonObject json;
    json.insert("id", "ORD-1001");
    json.insert("status", "CONFIRMED");
    json.insert("amount", 129.90);
    json.insert("customerId", "CUST-42");

    const Order order = Order::fromJson(json);

    EXPECT_EQ(order.id().toStdString(), "ORD-1001");
    EXPECT_EQ(order.status().toStdString(), "CONFIRMED");
    EXPECT_DOUBLE_EQ(order.amount(), 129.90);
    EXPECT_EQ(order.customerId().toStdString(), "CUST-42");
}

TEST(OrderTest, SerializeDeserializeRoundTrip) {
    const Order original("ORD-1002", "PENDING", 59.90, "CUST-7");
    const Order restored = Order::fromJson(original.toJson());
    EXPECT_EQ(original, restored);
}

TEST(OrderTest, CreateRequestOmitsIdAndStatusAndEmptyCustomer) {
    // Requête de création volontairement incomplète (pas de customerId) :
    // doit produire un JSON sans "id"/"status"/"customerId" pour déclencher la
    // réponse 400 du dispatcher JSON_BODY côté contrat.
    Order request;
    request.setAmount(0.0);

    const QJsonObject json = request.toJson();
    EXPECT_FALSE(json.contains("id"));
    EXPECT_FALSE(json.contains("status"));
    EXPECT_FALSE(json.contains("customerId"));
    EXPECT_TRUE(json.contains("amount"));
}
