import QtQuick 2.12
import QtQuick.Controls 2.12
import TabModel 1.0

//自定义QtQuick 2中的TableView
Item {
    id: control
    implicitHeight: 300
    implicitWidth: 500

    property int horHeaderHeight: 45

    //滚动条
    property color scrollBarColor: "cyan"
    property int scrollBarWidth: 6
    //列宽
    property variant columnWidthArr: [100,100,100,200]

    TableModel{
        id: table_model
        property var horHeader: ["监听端口","下游服务器","运行状态","启动/暂停"]
        property var initData: [
//            {"id":1,"name":"gonge","age":20,"note":"test model view"},
//            {"id":2,"name":"gonge","age":21,"note":"test model view"},
//            {"id":3,"name":"gonge","age":22,"note":"test model view"},
//            {"id":4,"name":"gonge","age":23,"note":"test model view"},
//            {"id":5,"name":"gonge","age":24,"note":"test model view"},
//            {"id":6,"name":"gonge","age":25,"note":"test model view"},
//            {"id":7,"name":"gonge","age":26,"note":"test model view"},
//            {"id":8,"name":"gonge","age":27,"note":"test model view"}
        ]
    }

    //表格内容（不包含表头）
    TableView{
        id: table_view
        anchors{
            fill: parent
            topMargin: control.horHeaderHeight
        }

        clip: true
        boundsBehavior: Flickable.StopAtBounds
        columnSpacing: 1
        rowSpacing: 1
        //视图的高度
        //contentHeight: rowHeightProvider(0) * rows + rowHeightProvider(rows-1)
        //视图的宽度
        //contentWidth:
        //content内容区域边距，但是不影响滚动条的位置
        //leftMargin:
        //topMargin:
        //此属性可以包含一个函数，该函数返回模型中每行的行高
//        rowHeightProvider: function (row) {
//            return control.verHeaderHeight;
//        }
        //此属性可以保存一个函数，该函数返回模型中每个列的列宽
        columnWidthProvider: function (column) {
//            if (1 === column){
//                return control.width / 8 * 5;
//            }
            return  control.width / 4;
        }
//        ScrollBar.vertical: ScrollBar {
//            id: scroll_vertical
//            anchors.right: parent.right
//            anchors.rightMargin: 2
//            //active: table_view.ScrollBar.vertical.active
//            //policy: ScrollBar.AsNeeded
//            contentItem: Rectangle{
//                visible: (scroll_vertical.size<1.0)
//                implicitWidth: control.scrollBarWidth
//                color: control.scrollBarColor
//            }
//        }

//        ScrollBar.horizontal: ScrollBar {
//            id: scroll_horizontal
//            anchors.bottom: parent.bottom
//            anchors.bottomMargin: 2
//            //active: table_view.ScrollBar.vertical.active
//            //policy: ScrollBar.AsNeeded
//            contentItem: Rectangle{
//                visible: (scroll_horizontal.size<1.0)
//                implicitHeight: control.scrollBarWidth
//                color: control.scrollBarColor
//            }
//        }

        //model是在C++中定义的，和QtC++是类似的
        model: table_model
        delegate: Rectangle{
            color: (model.row%2)?"orange":Qt.darker("orange")
//            TextInput{
//                anchors.fill: parent
//                verticalAlignment: Text.AlignVCenter
//                horizontalAlignment: Text.AlignHCenter
//                //elide: Text.ElideRight
//                selectByMouse: true
//                selectedTextColor: "black"
//                selectionColor: "white"

//                //获取单元格对应的值
//                text: model.value
//            }
        }
    }

    //横项表头
    Item{
        id: header_horizontal
        anchors{
            left: parent.left
            right: parent.right
        }
        height: control.horHeaderHeight
        width: control.width
        Row {
            id: header_horizontal_row
            anchors.fill: parent
            clip: true
            spacing: 0

            Repeater {
                model: table_model.horHeader

                Rectangle {
                    id: header_horizontal_item
                    width: table_view.columnWidthProvider(index)+table_view.columnSpacing
                    height: control.horHeaderHeight
                    color: "purple"

                    Text {
                        color: "white"
                        font.pixelSize: 20
                        font.bold: true
                        anchors.centerIn: parent
                        text: table_model.horHeader[index]
                    }
                }
            }
        }
    }
}
