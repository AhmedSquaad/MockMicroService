import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Commandes")
    Accessible.name: qsTr("Commandes")

    ListView {
        id: ordersView
        anchors.fill: parent
        model: ordersModel
        clip: true
        Accessible.name: qsTr("Liste des commandes")

        delegate: ItemDelegate {
            width: ordersView.width
            text: orderId + " — " + status + " (" + amount.toFixed(2) + " €) — " + customerId
            Accessible.name: text
        }
    }

    footer: ToolBar {
        Button {
            anchors.centerIn: parent
            text: qsTr("Rafraîchir")
            Accessible.name: qsTr("Rafraîchir")
            onClicked: ordersModel.refresh()
        }
    }
}
