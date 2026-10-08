#pragma once

#include <gmock/gmock.h>

#include "orders/network/IAPIClient.h"

namespace ecommerce::orders::test {

// Mock GMock de l'interface d'API : utilisé par les tests unitaires des classes
// qui consomment IApiClient (ex. OrdersListModel), sans toucher au réseau.
class MockApiClient : public IApiClient {
public:
    MOCK_METHOD(void, listOrders, (ListOrdersCallback callback), (override));
    MOCK_METHOD(void, getOrder, (const QString& orderId, OrderCallback callback), (override));
    MOCK_METHOD(void, createOrder, (const Order& request, OrderCallback callback), (override));
};

} // namespace ecommerce::orders::test
