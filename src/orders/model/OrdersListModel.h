#pragma once

#include <QAbstractListModel>
#include <QList>

#include "orders/model/Order.h"

namespace ecommerce::orders {

class IApiClient;

// Modèle de liste exposable à QML, alimenté par un IApiClient injecté.
// C'est la classe « consommatrice » de l'API testée unitairement avec MockAPIClient.
class OrdersListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        StatusRole,
        AmountRole,
        CustomerIdRole,
    };

    explicit OrdersListModel(IApiClient* apiClient, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    const QList<Order>& orders() const { return m_orders; }
    void setOrders(QList<Order> orders);

    Q_INVOKABLE void refresh();

signals:
    void loaded();
    void errorOccurred(const QString& message);

private:
    IApiClient* m_apiClient;
    QList<Order> m_orders;
};

} // namespace ecommerce::orders
