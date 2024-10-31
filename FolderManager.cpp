#include "FolderManager.h"

FolderManager::FolderManager(QObject *parent):
    QObject(parent),
    m_curFolder("file:///C://Users")
{}

// 目录树文件夹
QString FolderManager::curFolder() const{
    return m_curFolder;
}

void FolderManager::setCurFolder(const QString &newFolder){
    if(m_curFolder!=newFolder){
        m_curFolder=newFolder;
        emit curFolderChanged(m_curFolder);
    }
}

// 原始图片所在文件夹
QString FolderManager::pictureFolder() const{
    return m_pictureFolder;
}

void FolderManager::setPictureFolder(const QString &newFolder){
    if(m_pictureFolder!=newFolder){
        m_pictureFolder=newFolder;
        emit pictureFolderChanged(m_pictureFolder);
    }
}

// 结果输出的文件夹
QString FolderManager::resultFolder() const{
    return m_resultFolder;
}

void FolderManager::setResultFolder(const QString &newFolder){
    if(m_resultFolder!=newFolder){
        m_resultFolder=newFolder;
        emit resultFolderChanged(m_resultFolder);
    }
}
