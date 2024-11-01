#ifndef RUNNER_H
#define RUNNER_H

#include<QObject>
#include<QDebug>
#include <QStringList>
#include "FolderManager.h"

class Runner :public QObject {
    Q_OBJECT
        Q_PROPERTY(QString message READ getMessage WRITE setMessage NOTIFY messageChanged)

public:
    explicit Runner(FolderManager* folderManager, QObject* parent = nullptr)
        : QObject(parent), m_folderManager(folderManager) {}//, resolution(0.0), threshold(0.0), ssim(0.0), iteration_num(0) {}

    QString getMessage() const;
    void setMessage(const QString& newFolder);

    Q_INVOKABLE void updateLog(const QString& message);

public slots:
    void start(const QStringList& inputs);

signals:
    void finished(const QString& result);   // 任务完成信号
    //void updateLog(const QString &message);
    void messageChanged();

private:
    QString performTask(); // 实际执行的任务
    double resolution;
    double threshold;
    double ssim;
    int iteration_num;

    FolderManager* m_folderManager;

    QString m_message;
};

#endif // RUNNER_H
