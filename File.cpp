#include<QDebug>
#include<QFile>
#include <QTextStream>
#include "File.h"

File::File(){
    qDebug()<<"------File";
    connect(this,SIGNAL(sourceChanged()),this,SLOT(readFile()));
}
QString File::source() const{
    qDebug()<<"------source";
    return m_source;
}
void File::setSource(const QString &source){
    qDebug()<<"------setSource";
    m_source=source;
    emit sourceChanged();
}

QString File::text() const{
    qDebug()<<"------text";
    return m_text;
}

void File::setText(const QString &text) {
    qDebug() << "-------setText";
    QFile file(m_source);
    if (!file.open(QIODevice::WriteOnly)) {
        m_text = "";
        qDebug() << "Error: " << m_source << "open failed";
    } else {
        qint64 byte = file.write(text.toUtf8());
        if (byte != text.toUtf8().size()) {
            m_text = text.toUtf8().left(byte);
            qDebug() << "error:" << m_source << "open failed";
        } else {
            m_text = text;
        }
        file.close();
    }
    emit textChanged();
}


void File::readFile() {
    qDebug() << "-------readFile";
    QFile file(m_source);
    qDebug() << m_source;
    if (!file.open(QIODevice::ReadOnly)) {
        m_text = "";
        qDebug() << "error:" << m_source << "open failed";
    }
    // read from file ,save as m_text
    m_text = file.readAll();
    emit textChanged();
}

