import QtQuick 2.0

Rectangle {
    id:iconButton
    width:parent.width
    height:parent.height
    //用于外部调用时，输入图片的地址
    property string imageSource : image.source
    //添加信号
    signal buttonClicked()

    property color clr_enter: "#dcdcdc"
    property color clr_exit: "#ffffff"
    property color clr_click: "#aba9b2"
    property color clr_release: "#ffffff"

    //自定义点击信号
    signal clickedLeft()
    signal clickedRight()
    signal release()

    Image{
        id:image
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        clip:true
        fillMode: Image.PreserveAspectFit
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter
        source: imageSource
        onStatusChanged: {
            if (status === Image.Error) {
                console.log("Error loading image:", image.source);
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        //接受左键和右键输入
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            //左键点击
            if (mouse.button === Qt.LeftButton)
            {
                parent.clickedLeft()
                //console.log(button.text + " Left button click")
            }
            else if(mouse.button === Qt.RightButton)
            {
                parent.clickedRight()
                //console.log(button.text + " Right button click")
            }
        }

        //按下
        onPressed: {
            color = clr_click
        }

        //释放
        onReleased: {
            color = clr_enter
            parent.release()
            console.log("Release")
        }

        //指针进入
        onEntered: {
            color = clr_enter
            //console.log(button.text + " mouse entered")
        }

        //指针退出
        onExited: {
            color = clr_exit
            //console.log(button.text + " mouse exited")
        }
    }
}
