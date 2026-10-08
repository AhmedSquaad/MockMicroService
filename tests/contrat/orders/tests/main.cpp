#include <QCoreApplication>

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    // QCoreApplication requis : RestApiClient repose sur QNetworkAccessManager
    // et la boucle d'événements Qt (voir ContractTestSupport::waitUntil).
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
