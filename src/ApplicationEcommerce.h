#pragma once

#include <QString>
#include <memory>

#include <QObject>

class QQmlApplicationEngine;

namespace ecommerce {

namespace orders {
class RestApiClient;
class OrdersListModel;
} // namespace orders

// Compose l'application : crée le client REST (pointant vers le µService ou son
// mock), le modèle de commandes, les expose à QML et charge l'UI.
class ApplicationEcommerce : public QObject {
    Q_OBJECT
public:
    explicit ApplicationEcommerce(QString apiBaseUrl, QObject* parent = nullptr);
    ~ApplicationEcommerce() override;

    bool start();

private:
    QString m_apiBaseUrl;
    std::unique_ptr<orders::RestApiClient> m_apiClient;
    std::unique_ptr<orders::OrdersListModel> m_ordersModel;
    std::unique_ptr<QQmlApplicationEngine> m_engine;
};

} // namespace ecommerce
