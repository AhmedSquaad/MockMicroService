#include "OrdersListModel.h"

#include "orders/network/IAPIClient.h"

namespace ecommerce::orders {

OrdersListModel::OrdersListModel(IApiClient* apiClient, QObject* parent)
    : QAbstractListModel(parent), m_apiClient(apiClient) {}

int OrdersListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_orders.size());
}

QVariant OrdersListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_orders.size())
        return {};
    const Order& order = m_orders.at(index.row());
    switch (role) {
        case IdRole:
            return order.id();
        case StatusRole:
            return order.status();
        case AmountRole:
            return order.amount();
        case CustomerIdRole:
            return order.customerId();
        default:
            return {};
    }
}

QHash<int, QByteArray> OrdersListModel::roleNames() const {
    return {
        {IdRole, "orderId"},
        {StatusRole, "status"},
        {AmountRole, "amount"},
        {CustomerIdRole, "customerId"},
    };
}

void OrdersListModel::setOrders(QList<Order> orders) {
    beginResetModel();
    m_orders = std::move(orders);
    endResetModel();
}

void OrdersListModel::refresh() {
    if (!m_apiClient)
        return;
    m_apiClient->listOrders([this](const QList<Order>& orders, const ApiError& error) {
        if (error.failed) {
            emit errorOccurred(error.message);
            return;
        }
        setOrders(orders);
        emit loaded();
    });
}

} // namespace ecommerce::orders
