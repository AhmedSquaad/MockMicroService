#include "RestApiClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace ecommerce::orders {

namespace {

ApiError errorFromReply(QNetworkReply* reply) {
    const int status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() == QNetworkReply::NoError)
        return ApiError::ok(status);
    return ApiError::error(status, reply->errorString());
}

} // namespace

RestApiClient::RestApiClient(QString baseUrl, QObject* parent)
    : RestApiClient(std::move(baseUrl), new QNetworkAccessManager, parent) {
    m_manager->setParent(this);
}

RestApiClient::RestApiClient(QString baseUrl, QNetworkAccessManager* manager, QObject* parent)
    : QObject(parent), m_baseUrl(std::move(baseUrl)), m_manager(manager) {}

void RestApiClient::listOrders(ListOrdersCallback callback) {
    const QNetworkRequest request{QUrl(m_baseUrl + QStringLiteral("/orders"))};
    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this,
            [reply, callback = std::move(callback)]() {
                reply->deleteLater();
                const ApiError error = errorFromReply(reply);
                QList<Order> orders;
                if (!error.failed) {
                    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                    const QJsonArray array = doc.array();
                    orders.reserve(array.size());
                    for (const QJsonValue& value : array)
                        orders.append(Order::fromJson(value.toObject()));
                }
                callback(orders, error);
            });
}

void RestApiClient::getOrder(const QString& orderId, OrderCallback callback) {
    const QNetworkRequest request{
        QUrl(m_baseUrl + QStringLiteral("/orders/") + orderId)};
    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this,
            [reply, callback = std::move(callback)]() {
                reply->deleteLater();
                const ApiError error = errorFromReply(reply);
                Order order;
                if (!error.failed)
                    order = Order::fromJson(
                        QJsonDocument::fromJson(reply->readAll()).object());
                callback(order, error);
            });
}

void RestApiClient::createOrder(const Order& request, OrderCallback callback) {
    QNetworkRequest httpRequest{QUrl(m_baseUrl + QStringLiteral("/orders"))};
    httpRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json"));
    const QByteArray body =
        QJsonDocument(request.toJson()).toJson(QJsonDocument::Compact);
    QNetworkReply* reply = m_manager->post(httpRequest, body);
    connect(reply, &QNetworkReply::finished, this,
            [reply, callback = std::move(callback)]() {
                reply->deleteLater();
                const ApiError error = errorFromReply(reply);
                Order created;
                if (!error.failed)
                    created = Order::fromJson(
                        QJsonDocument::fromJson(reply->readAll()).object());
                callback(created, error);
            });
}

} // namespace ecommerce::orders
