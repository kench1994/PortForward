import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls.Material 2.3

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("端口转发")

    Material.theme: Material.Dark
    Material.accent: Material.Purple

    color: "#121624"

    Rectangle{
        id: table
        anchors.fill: parent
        anchors.bottomMargin: 100
        anchors.margins: 20
        color: "gray"

        QTable{
            anchors.fill: parent
        }
    }


    AddPlane {
        anchors.left: Window.left
        anchors.right: Window.right
        anchors.top: table.bottom
        anchors.margins: 20

        width: Window.width
        height: 60

    }
}
