#ifndef FOLDERMANAGER_H
#define FOLDERMANAGER_H

#include<QString>
#include<QObject>

class FolderManager :public QObject {
    Q_OBJECT
        Q_PROPERTY(QString curFolder READ curFolder WRITE setCurFolder NOTIFY curFolderChanged) //目录树
        Q_PROPERTY(QString baseFolder READ baseFolder WRITE setBaseFolder NOTIFY baseFolderChanged) // 图片文件夹
        Q_PROPERTY(QString resultFolder READ resultFolder WRITE setResultFolder NOTIFY resultFolderChanged)     //结果文件夹


public:
    explicit FolderManager(QObject* parent = nullptr);

    QString curFolder() const;
    void setCurFolder(const QString& newFolder);

    QString baseFolder() const;
    void setBaseFolder(const QString& newFolder);

    QString resultFolder() const;
    void setResultFolder(const QString& newFolder);


signals:
    void curFolderChanged(const QString& newFolder);
    void baseFolderChanged(const QString& newFolder);
    void resultFolderChanged(const QString& newFolder);

private:
    QString m_curFolder;
    QString m_baseFolder;
    QString m_resultFolder;
};

#endif // FOLDERMANAGER_H
