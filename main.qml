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
    //title: qsTr("��˿����ָ�")

    property int curSelected: -1
    property string curPath:""

    property bool openText:false
    property bool openPic:false
    property bool openExcel:false

    ColumnLayout {
        anchors.fill: parent        // �������� ColumnLayout ������������

        Input{
            id: input
            Layout.alignment: Qt.AlignCenter
            property string curFolder: value
        }

        RowLayout{
            Layout.preferredHeight:parent.height-input.height
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignLeft

            Rectangle{      // Ŀ¼��
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
                        border.width:(curSelected==index)?2:0.5   // ��ѡ�е���߿�Ӵ�
                        border.color:(curSelected==index)? "lightblue" : "black" // ѡ��ʱ�ı�߿���ɫ
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
                                    // ��ȷ���ı��ѳ�ʼ��
                                    if (fileText.width > rectangleWidth) {
                                        tooltip.text = fileName; // �����������ļ���
                                        tooltip.visible = true; // ��ʾ ToolTip
                                        tooltip.x = mouse.x; // ���� ToolTip �� X λ��
                                        tooltip.y = mouse.y; // ���� ToolTip �� Y λ��
                                    }
                                }
                                onExited: {
                                    tooltip.visible = false; // ���� ToolTip
                                }
                                onClicked:{             // �����¼�
                                    console.log(curSelected,index)
                                    curSelected=index
                                    curPath=folderListModel.get (index, "fileURL")
                                }
                                onDoubleClicked: {      // ˫���¼�
                                    var folderPath=folderListModel.get (index, "fileURL")
                                    if (folderListModel.isFolder(index)) {
                                        console.log("����һ���ļ���: " + fileName)
                                        // ȥ�����ܵ�β��б�ܣ���ѡ��
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

                                        console.log("�ⲻ��һ���ļ���: " + fileName)
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

                    Rectangle{      // ͼƬ/�ļ�չʾ��
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
                                    console.log("��ǰ����: " + text)
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
                    Rectangle{      // ��������
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
                Rectangle{      // ��������
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
                        onMessageChanged:{           // ����runner��update
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
