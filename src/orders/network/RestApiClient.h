#pragma once

#include <QObject>
#include <QString>

#include "orders/network/IAPIClient.h"

class QNetworkAccessManager;

namespace ecommerce::orders {

// Implémentation réelle d'IApiClient au-dessus de QNetworkAccessManager.
// C'est cette classe qui est validée par les tests de contrat contre le mock Microcks.
class RestApiClient : public QObject, public IApiClient {
    Q_OBJECT
public:
    explicit RestApiClient(QString baseUrl, QObject* parent = nullptr);
    // Permet d'injecter un QNetworkAccessManager (tests avancés / partage).
    RestApiClient(QString baseUrl, QNetworkAccessManager* manager, QObject* parent);

    void listOrders(ListOrdersCallback callback) override;
    void getOrder(const QString& orderId, OrderCallback callback) override;
    void createOrder(const Order& request, OrderCallback callback) override;

private:
    QString m_baseUrl;
    QNetworkAccessManager* m_manager;
};

} // namespace ecommerce::orders
