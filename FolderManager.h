#ifndef FOLDERMANAGER_H
#define FOLDERMANAGER_H

#include<QString>
#include<QObject>

class FolderManager:public QObject{
    Q_OBJECT
    Q_PROPERTY(QString curFolder READ curFolder WRITE setCurFolder NOTIFY curFolderChanged) //目录树
    Q_PROPERTY(QString pictureFolder READ pictureFolder WRITE setPictureFolder NOTIFY pictureFolderChanged) // 图片文件夹
    Q_PROPERTY(QString resultFolder READ resultFolder WRITE setResultFolder NOTIFY resultFolderChanged)     //结果文件夹


public:
    explicit FolderManager(QObject *parent = nullptr);

    QString curFolder() const;
    void setCurFolder(const QString &newFolder);

    QString pictureFolder() const;
    void setPictureFolder(const QString &newFolder);

    QString resultFolder() const;
    void setResultFolder(const QString &newFolder);


signals:
    void curFolderChanged(const QString &newFolder);
    void pictureFolderChanged(const QString &newFolder);
    void resultFolderChanged(const QString &newFolder);

private:
    QString m_curFolder;
    QString m_pictureFolder;
    QString m_resultFolder;
};

#endif // FOLDERMANAGER_H
