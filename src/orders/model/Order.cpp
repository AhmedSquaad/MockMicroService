#include "Order.h"

#include <QtGlobal>

namespace ecommerce::orders {

Order::Order(QString id, QString status, double amount, QString customerId)
    : m_id(std::move(id))
    , m_status(std::move(status))
    , m_amount(amount)
    , m_customerId(std::move(customerId)) {}

Order Order::fromJson(const QJsonObject& json) {
    Order order;
    order.m_id = json.value(QStringLiteral("id")).toString();
    order.m_status = json.value(QStringLiteral("status")).toString();
    order.m_amount = json.value(QStringLiteral("amount")).toDouble();
    order.m_customerId = json.value(QStringLiteral("customerId")).toString();
    return order;
}

QJsonObject Order::toJson() const {
    QJsonObject json;
    // id/status/customerId ne sont émis que s'ils sont renseignés : une requête de
    // création volontairement incomplète (customerId absent) doit pouvoir être
    // produite pour exercer le dispatcher JSON_BODY du contrat (réponse 400).
    if (!m_id.isEmpty())
        json.insert(QStringLiteral("id"), m_id);
    if (!m_status.isEmpty())
        json.insert(QStringLiteral("status"), m_status);
    json.insert(QStringLiteral("amount"), m_amount);
    if (!m_customerId.isEmpty())
        json.insert(QStringLiteral("customerId"), m_customerId);
    return json;
}

bool Order::operator==(const Order& other) const {
    return m_id == other.m_id
        && m_status == other.m_status
        && qFuzzyCompare(m_amount, other.m_amount)
        && m_customerId == other.m_customerId;
}

} // namespace ecommerce::orders
