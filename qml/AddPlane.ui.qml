import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12


Rectangle  {
    id: addPlane
    color: "transparent"

    radius: 50

    RoundButton  {
        text: "+"
        radius: 50
        width: 65
        font.pixelSize: 50
        anchors.centerIn: addPlane
    }
}
