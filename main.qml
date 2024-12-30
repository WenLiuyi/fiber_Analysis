import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.folderlistmodel 2.1
import MyFile

Window {
    width: 1280
    height: 720
    visible: true
    //title: qsTr("单丝截面分割")

    property int curSelected: -1
    property string curPath:""

    property bool openText:false
    property bool openPic:false
    property bool openExcel:false

    ColumnLayout {
        anchors.fill: parent        // 将最外层的 ColumnLayout 布满整个窗口

        Input{
            id: input
            Layout.alignment: Qt.AlignCenter
            property string curFolder: value
        }

        RowLayout{
            Layout.preferredHeight:parent.height-input.height
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignLeft

            Rectangle{      // 目录栏
                id:directoryTree
                anchors.left: parent.left
                Layout.preferredWidth:parent.width*0.15
                Layout.fillHeight: true;//Layout.fillWidth: true

                ListView{
                    id:listView
                    anchors.fill:parent
                    clip:true

                    model:FolderListModel{
                        id:folderListModel
                        showDirs:true
                        showDotAndDotDot: false
                        showFiles: true

                        Component.onCompleted: {
                            for (var i = 0; i < folderListModel.count; i++) {
                                isSelectedModel.append({isSelected:false});
                            }
                        }
                    }
                    delegate:Rectangle{
                        id:dirItem
                        anchors.horizontalCenter: parent.horizontalCenter
                        width:parent.width
                        height:30
                        border.width:(curSelected==index)?2:0.5   // 被选中的项边框加粗
                        border.color:(curSelected==index)? "lightblue" : "black" // 选中时改变边框颜色
                        //radius:5

                        Text{
                            id:fileText
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 5
                            text:fileName
                            color: folderListModel.isFolder(index)? "black" : "gray"
                            font.pixelSize: 12
                            elide:Text.ElideRight

                            ToolTip{
                                id: tooltip
                                visible: false
                            }
                            MouseArea {
                                id:mouse
                                anchors.fill: parent
                                hoverEnabled: true
                                width:parent.width
                                property real rectangleWidth:parent.parent.parent.parent.width

                                onEntered: {
                                    //console.log(fileText.width,rectangleWidth)
                                    // 先确保文本已初始化
                                    if (fileText.width > rectangleWidth) {
                                        tooltip.text = fileName; // 设置完整的文件名
                                        tooltip.visible = true; // 显示 ToolTip
                                        tooltip.x = mouse.x; // 设置 ToolTip 的 X 位置
                                        tooltip.y = mouse.y; // 设置 ToolTip 的 Y 位置
                                    }
                                }
                                onExited: {
                                    tooltip.visible = false; // 隐藏 ToolTip
                                }
                                onClicked:{             // 单击事件
                                    console.log(curSelected,index)
                                    curSelected=index
                                    curPath=folderListModel.get (index, "fileURL")
                                }
                                onDoubleClicked: {      // 双击事件
                                    var folderPath=folderListModel.get (index, "fileURL")
                                    if (folderListModel.isFolder(index)) {
                                        console.log("这是一个文件夹: " + fileName)
                                        // 去掉可能的尾随斜杠（可选）
                                        /*if (folderPath.endsWith('/')) {
                                            folderPath = folderPath.slice(0, -1);
                                        }*/
                                        folderManager.curFolder=folderPath
                                        folderListModel.folder= folderPath

                                    } else {
                                        if(fileName.endsWith('.txt')){
                                            file.source=folderPath.toString().slice(8)
                                            openText=true;openPic=false;openExcel=false
                                        }else if(fileName.endsWith('.xlsx')){

                                        }else if(fileName.endsWith('.png')||file.endsWith('.jpg')){
                                            imageViewer.source = folderPath
                                            openText=false;openPic=true;openExcel=false
                                        }

                                        console.log("这不是一个文件夹: " + fileName)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout{
                Layout.fillWidth: true
                Layout.fillHeight: true
                RowLayout{
                    Layout.fillWidth: true
                    Layout.preferredHeight: 370

                    Rectangle{      // 图片/文件展示栏
                        anchors.top:parent.top
                        Layout.preferredWidth: 672
                        Layout.fillHeight: true

                        File{
                            id:file
                        }

                        ScrollView {
                            id:textScroll
                            width: parent.width
                            height: parent.height

                            TextArea {
                                id: textArea_3
                                visible:openText
                                width: 672  
                                height: parent.height
                                text:file.text

                                onTextChanged: {
                                    console.log("当前输入: " + text)
                                }
                                background: Rectangle {
                                    border.color: "#B0B0B0"
                                    radius: 3
                                }
                            }
                            Image{
                                id:imageViewer
                                fillMode: Image.PreserveAspectFit
                                visible:openPic
                                width: 672
                                source:""
                            }
                        }
                    }
                    Rectangle{      // 工作区栏
                        anchors.top:parent.top
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ScrollView{
                            id:commandScroll
                            width:parent.width;height:parent.height

                            TextArea{
                            id:paramsArea
                            anchors.fill: parent
                            height: parent.height
                            readOnly: true

                            background: Rectangle {
                                border.color: "#B0B0B0"
                                radius: 3
                            }
                        }
                        }
                        
                        
                    }
                }
                Rectangle{      // 命令行栏
                    color: "lightskyblue";
                    Layout.fillWidth: true
                    Layout.preferredHeight: 230

                    ScrollView {
                        width: parent.width
                        height: parent.height

                        TextArea {
                            id: logArea
                            anchors.fill: parent
                            readOnly: true

                            background: Rectangle {
                                border.color: "#B0B0B0"
                                radius: 3
                            }
                        }
                    }

                    
                    Connections{
                        target:runner
                        onMessageChanged:{           // 连接runner的update
                            console.log("hello")
                            console.log(runner.message)
                            logArea.text += runner.message + "\n";
                            //logArea.scrollToBottom();
                        }
                    }
                }
            }
        }
    }

}
