import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 480
    height: 640
    visible: true
    title: qsTr("E-commerce — Commandes")

    // Le panneau du module "orders" consomme le contexte `ordersModel` exposé
    // par ApplicationEcommerce.
    Loader {
        anchors.fill: parent
        source: "qrc:/orders/qml/OrdersPanel.qml"
    }
}
