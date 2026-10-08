#pragma once

#include <QCoreApplication>
#include <QEventLoop>
#include <QString>
#include <QTimer>

#include <QByteArray>

namespace ecommerce::orders::test {

// URL de base du mock REST. Surchargée en CI via MOCK_BASE_URL (ex. le service
// GitLab `microcks`), défaut local = instance docker-compose de ce dossier.
inline QString mockBaseUrl() {
    const QByteArray env = qgetenv("MOCK_BASE_URL");
    return env.isEmpty()
               ? QStringLiteral("http://localhost:8585/rest/Order+Service+API/1.0.0")
               : QString::fromUtf8(env);
}

// Fait tourner la boucle d'événements Qt jusqu'à ce que `done` passe à true
// (callback réseau arrivé) ou que le délai expire. Retourne la valeur de `done`.
inline bool waitUntil(const bool& done, int timeoutMs = 15000) {
    QEventLoop loop;
    QTimer deadline;
    deadline.setSingleShot(true);
    QObject::connect(&deadline, &QTimer::timeout, &loop, &QEventLoop::quit);

    QTimer poll;
    QObject::connect(&poll, &QTimer::timeout, &loop, [&]() {
        if (done)
            loop.quit();
    });

    deadline.start(timeoutMs);
    poll.start(25);
    loop.exec();
    return done;
}

} // namespace ecommerce::orders::test
