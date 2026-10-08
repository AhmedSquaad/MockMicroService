#pragma once

#include <functional>

#include <QList>
#include <QString>

#include "orders/model/Order.h"

namespace ecommerce::orders {

// Résultat d'un appel réseau : statut HTTP + éventuel message d'erreur.
struct ApiError {
    bool failed = false;
    int httpStatus = 0;
    QString message;

    static ApiError ok(int status = 200) { return {false, status, {}}; }
    static ApiError error(int status, QString message) {
        return {true, status, std::move(message)};
    }
};

using ListOrdersCallback = std::function<void(const QList<Order>&, const ApiError&)>;
using OrderCallback = std::function<void(const Order&, const ApiError&)>;

// Interface d'accès à l'API du µService commandes. Asynchrone (le résultat est
// délivré via callback), ce qui la rend à la fois idiomatique côté Qt et
// mockable par GMock pour les tests unitaires (voir tests/unit/.../MockAPIClient.h).
class IApiClient {
public:
    virtual ~IApiClient() = default;

    virtual void listOrders(ListOrdersCallback callback) = 0;
    virtual void getOrder(const QString& orderId, OrderCallback callback) = 0;
    virtual void createOrder(const Order& request, OrderCallback callback) = 0;
};

} // namespace ecommerce::orders
