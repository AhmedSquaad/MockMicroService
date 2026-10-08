#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QString>

#include "mocks/MockAPIClient.h"
#include "orders/model/OrdersListModel.h"

using namespace ecommerce::orders;
using ::testing::_;
using ::testing::Invoke;

TEST(OrdersListModelTest, RefreshPopulatesModelFromApiClient) {
    test::MockApiClient apiClient;
    EXPECT_CALL(apiClient, listOrders(_))
        .WillOnce(Invoke([](ListOrdersCallback callback) {
            callback({Order("ORD-1001", "CONFIRMED", 129.90, "CUST-42"),
                      Order("ORD-1002", "PENDING", 59.90, "CUST-7")},
                     ApiError::ok());
        }));

    OrdersListModel model(&apiClient);
    model.refresh();

    ASSERT_EQ(model.rowCount(), 2);
    EXPECT_EQ(model.orders().at(0).id().toStdString(), "ORD-1001");
    EXPECT_EQ(model.orders().at(1).status().toStdString(), "PENDING");
}

TEST(OrdersListModelTest, RefreshEmitsErrorOnApiFailure) {
    test::MockApiClient apiClient;
    EXPECT_CALL(apiClient, listOrders(_))
        .WillOnce(Invoke([](ListOrdersCallback callback) {
            callback({}, ApiError::error(500, "internal error"));
        }));

    OrdersListModel model(&apiClient);
    // Connexion directe (même thread, pas de boucle d'événements) : le signal est
    // émis de façon synchrone pendant refresh(), inutile de dépendre de Qt6::Test.
    QString lastError;
    int errorCount = 0;
    QObject::connect(&model, &OrdersListModel::errorOccurred,
                     [&](const QString& message) {
                         lastError = message;
                         ++errorCount;
                     });

    model.refresh();

    EXPECT_EQ(model.rowCount(), 0);
    ASSERT_EQ(errorCount, 1);
    EXPECT_EQ(lastError.toStdString(), "internal error");
}
