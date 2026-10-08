#include "ApplicationEcommerce.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include "orders/model/OrdersListModel.h"
#include "orders/network/RestApiClient.h"

namespace ecommerce {

using orders::OrdersListModel;
using orders::RestApiClient;

ApplicationEcommerce::ApplicationEcommerce(QString apiBaseUrl, QObject* parent)
    : QObject(parent), m_apiBaseUrl(std::move(apiBaseUrl)) {}

ApplicationEcommerce::~ApplicationEcommerce() = default;

bool ApplicationEcommerce::start() {
    m_apiClient = std::make_unique<RestApiClient>(m_apiBaseUrl);
    m_ordersModel = std::make_unique<OrdersListModel>(m_apiClient.get());
    m_engine = std::make_unique<QQmlApplicationEngine>();

    m_engine->rootContext()->setContextProperty(QStringLiteral("ordersModel"),
                                                m_ordersModel.get());
    m_ordersModel->refresh();

    m_engine->load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    return !m_engine->rootObjects().isEmpty();
}

} // namespace ecommerce
