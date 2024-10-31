import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
import QtQuick.Dialogs
ColumnLayout{
    id:input
    anchors.top:parent.top
    Layout.preferredWidth: parent.width;Layout.preferredHeight:parent.height/6
    //width:parent.width;height:parent.height/6;//color:'lightgrey'
    spacing:0

    signal curFolderChanged(string newFolder)

    RowLayout{
        id: menuItem
        spacing: 0
        Layout.alignment: Qt.AlignVCenter
        Layout.fillHeight: true;Layout.fillWidth: true

        Button{
            text:"文件"
            font.pixelSize: 16
            onClicked:{switchToolBar("file")}
        }
        Button{
            text:"参数设置"
            font.pixelSize: 16
            onClicked:{switchToolBar("edit")}
        }
        Button{
            text:"Back"
            font.pixelSize: 16
            onClicked: {
                var head=folderManager.curFolder.substring(0,8) // "file:///"
                var tail=folderManager.curFolder.substring(8)
                var parts=tail.split("/")
                parts.pop()
                var trimmedPath = parts.join("/")
                trimmedPath=head+trimmedPath

                folderListModel.folder=trimmedPath
                folderManager.curFolder=trimmedPath
            }
        }
        Button{
            text:"To"
            font.pixelSize: 16
            onClicked:{
                if(curSelected>=0){
                    if (folderListModel.isFolder(curSelected)) {
                        folderListModel.folder= curPath

                    } else {
                        console.log("这不是一个文件夹: " + fileName)
                    }
                }
            }
        }
        Button{
            text:"Start"
            font.pixelSize: 16
            onClicked:{
                let inputs=[resolutionInput.text,thresholdInput.text,ssimInput.text];
                let allValid=true;

                if(!isDouble(resolutionInput.text)){
                    allValid=false;
                    inputBorder_1.border.color = "red";
                }else{
                    inputBorder_1.border.color = "lightgray";
                }

                if(!isDouble(thresholdInput.text)){
                    allValid=false;
                    inputBorder_2.border.color = "red";
                }else{
                    inputBorder_2.border.color = "lightgray";
                }

                if(!isDouble(ssimInput.text)){
                    allValid=false;
                    inputBorder_3.border.color = "red";
                }else{
                    inputBorder_3.border.color = "lightgray";
                }
                if(!isInt(iternumInput.text)) {
                    allValid=false;
                    inputBorder_4.border.color="red";
                }else{
                    inputBorder_4.border.color="lightgray";
                }

                if(allValid) runner.run();   // 调用Runner的run方法
            }
        }Connections{
            target:runner
            onFinished:{

            }
        }

        Button{
            text:"Stop"
            font.pixelSize: 16
            onClicked:{

            }
        }

        IconButton{
            id:runButton
            imageSource:"icon/run_icon.png"
        }
        IconButton{
            id:stopButton
            imageSource:"icon/run_icon.png"
        }
    }
    RowLayout{
        spacing:800
        Layout.alignment: Qt.AlignVCenter
        Layout.fillHeight: true;Layout.fillWidth: true

        ColumnLayout{
            id: filesSet;visible:true;spacing:50
            Layout.alignment:Qt.AlignTop
            Item{
                ScrollView {
                    Layout.alignment:Qt.AlignTop
                    id:inputScroll
                    width: 700
                    height: 45  // 设置固定高度
                    x:parent.left+5

                    TextArea {
                        id: textArea_1
                        width: 500  // 设置输入框宽度超过 ScrollView 的宽度
                        placeholderText: "图片路径"
                        wrapMode: TextArea.Wrap.None
                        onTextChanged: {
                            console.log("当前输入: " + text)
                        }
                        background: Rectangle {
                            border.color: "#B0B0B0"
                            radius: 3
                        }
                    }
                }
                Button{
                    id:inputButton
                    height:45
                    width:60
                    anchors.left:inputScroll.right
                    text:"选择";onClicked:inputFileDialog.open()
                }

                FolderDialog{
                    id:inputFileDialog
                    acceptLabel: "确定"
                    rejectLabel: "取消"
                    title: "选择图片所在的文件夹"
                    onAccepted: {
                        textArea_1.text = currentFolder  // 将选择的文件夹路径设置到 textArea
                    }
                    onCurrentFolderChanged: {
                        folderListModel.folder=currentFolder
                        folderManager.curFolder=currentFolder       // 目录树路径
                        folderManager.pictureFolder=currentFolder   // 图片路径
                    }
                }
            }
            Item{
                ScrollView {
                    id:outputScroll
                    width: 700
                    height: 45
                    x:parent.left+5

                    TextArea {
                        id: textArea_2
                        width: 500  // 设置输入框宽度超过 ScrollView 的宽度
                        placeholderText: "输出路径"
                        wrapMode: TextArea.Wrap.None  // 禁止换行
                        onTextChanged: {
                            console.log("当前输入: " + text)
                        }
                        background: Rectangle {
                            border.color: "#B0B0B0"
                            radius: 3
                        }
                    }
                }
                Button{
                    id:outputButton
                    height:45
                    width:60
                    anchors.left:outputScroll.right
                    text:"选择";onClicked:outputFileDialog.open()
                }

                FolderDialog{
                    id:outputFileDialog
                    acceptLabel: "确定"
                    rejectLabel: "取消"
                    currentFolder: selectedFolder
                    title: "选择图片所在的文件夹"
                    onAccepted: {
                        textArea_2.text = currentFolder  // 将选择的文件夹路径设置到 textArea
                    }
                    onCurrentFolderChanged: {
                        folderListModel.folder=currentFolder
                        folderManager.curFolder=currentFolder       // 目录树路径
                        folderManager.pictureFolder=currentFolder   // 输出路径
                    }
                }
            }
        }
        ColumnLayout{
            id: paramsSet;visible:true
            spacing:0
            Layout.alignment: Qt.AlignVCenter
            Layout.fillHeight: true;Layout.fillWidth: true

            RowLayout{
                id:resolutionLayout
                spacing:5
                Layout.alignment: Qt.AlignVCenter
                Layout.fillHeight: true;Layout.fillWidth: true
                // 标签
                Text {
                    text: "图像分辨率：";width:80
                    font.pointSize: 10.5  // 设置字体大小
                    verticalAlignment: Text.AlignVCenter  // 垂直居中对齐
                }
                // 输入框
                TextField {
                    id: resolutionInput  // 给输入框一个ID以便引用
                    width: 500  // 设置输入框的宽度
                    font.pointSize: 10.5
                    background:Rectangle{
                        id:inputBorder_1
                        border.color: "lightgray"
                        width:100
                    }
                }
            }
            RowLayout{
                id:thresholdLayout
                spacing:5
                Layout.alignment: Qt.AlignVCenter
                Layout.fillHeight: true;Layout.fillWidth: true
                Text {
                    text: "图像再分割阈值系数：";width:80
                    font.pointSize: 10.5
                    verticalAlignment: Text.AlignVCenter
                }
                TextField {
                    id: thresholdInput
                    width: 500
                    font.pointSize: 10.5
                    background:Rectangle{
                        id:inputBorder_2
                        border.color: "lightgray"
                        width:100
                    }
                }
            }
            RowLayout{
                id:ssimLayout
                spacing:5
                Layout.alignment: Qt.AlignVCenter
                Layout.fillHeight: true;Layout.fillWidth: true
                Text {
                    text: "图像结构相似性阈值：";width:80
                    font.pointSize: 10.5
                    verticalAlignment: Text.AlignVCenter
                }
                TextField {
                    id: ssimInput
                    width: 500
                    font.pointSize: 10.5
                    background:Rectangle{
                        id:inputBorder_3
                        border.color: "lightgray"
                        width:100
                    }
                }
            }
            RowLayout{
                id:iternumLayout
                spacing:5
                Layout.alignment: Qt.AlignVCenter
                Layout.fillHeight: true;Layout.fillWidth: true
                Text {
                    text: "消除随机性分割错误迭代次数：";width:80
                    font.pointSize: 10.5
                    verticalAlignment: Text.AlignVCenter
                }
                TextField {
                    id: iternumInput
                    width: 500
                    font.pointSize: 10.5
                    background:Rectangle{
                        id:inputBorder_4
                        border.color: "lightgray"
                        width:100
                    }
                }
            }
        }
    }
    // 切换工具栏内容的函数
    function switchToolBar(type) {
        fileTool.visible = false;
        editTool.visible = false;

        if (type === "file") {
            fileTool.visible = true;
        } else if (type === "edit") {
            editTool.visible = true;
        } else {
            fileTool.visible = true;
        }
    }
    function isDouble(value){
        return !isNaN(value) && value.trim() !== "" && !isNaN(parseFloat(value));
    }
    function isInt(value){
        return !isNaN(value) && value.trim() !== "" && !isNaN(parseInt(value));
    }
}


