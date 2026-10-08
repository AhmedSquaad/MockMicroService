#pragma once

#include <QJsonObject>
#include <QString>

namespace ecommerce::orders {

// DTO d'une commande : porte les champs du contrat OpenAPI (order-service.yaml)
// et sait (dé)sérialiser en JSON via les API Qt (QJsonObject).
class Order {
public:
    Order() = default;
    Order(QString id, QString status, double amount, QString customerId);

    const QString& id() const { return m_id; }
    void setId(QString id) { m_id = std::move(id); }

    const QString& status() const { return m_status; }
    void setStatus(QString status) { m_status = std::move(status); }

    double amount() const { return m_amount; }
    void setAmount(double amount) { m_amount = amount; }

    const QString& customerId() const { return m_customerId; }
    void setCustomerId(QString customerId) { m_customerId = std::move(customerId); }

    static Order fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

    bool operator==(const Order& other) const;
    bool operator!=(const Order& other) const { return !(*this == other); }

private:
    QString m_id;
    QString m_status;
    double m_amount = 0.0;
    QString m_customerId;
};

} // namespace ecommerce::orders
