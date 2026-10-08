#include <QByteArray>
#include <QGuiApplication>
#include <QString>

#include "ApplicationEcommerce.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    // URL de base de l'API : le µService réel en production, son mock Microcks
    // en dev/CI. Surchargée par la variable d'environnement MOCK_BASE_URL.
    const QByteArray envBaseUrl = qgetenv("MOCK_BASE_URL");
    const QString apiBaseUrl =
        envBaseUrl.isEmpty()
            ? QStringLiteral("http://localhost:8585/rest/Order+Service+API/1.0.0")
            : QString::fromUtf8(envBaseUrl);

    ecommerce::ApplicationEcommerce application(apiBaseUrl);
    if (!application.start())
        return -1;

    return app.exec();
}
