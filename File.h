#ifndef FILE_H
#define FILE_H
#pragma execution_character_set("utf-8")

#include<QObject>
class File:public QObject{
    Q_OBJECT

public:
    File();
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

    QString source() const;
    void setSource(const QString &source);
    QString text() const;
    void setText(const QString &text);

signals:
    void sourceChanged();
    void textChanged();

private slots:
    void readFile();

private:
    QString m_source;
    QString m_text;
};

#endif // FILE_H
