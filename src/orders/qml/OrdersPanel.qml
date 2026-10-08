import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Commandes")

    ListView {
        id: ordersView
        anchors.fill: parent
        model: ordersModel
        clip: true

        delegate: ItemDelegate {
            width: ordersView.width
            text: orderId + " — " + status + " (" + amount.toFixed(2) + " €) — " + customerId
        }
    }

    footer: ToolBar {
        Button {
            anchors.centerIn: parent
            text: qsTr("Rafraîchir")
            onClicked: ordersModel.refresh()
        }
    }
}
