#ifndef RUNNER_H
#define RUNNER_H

#include<QObject>
#include<QDebug>
#include <QStringList>
#include "FolderManager.h"

class Runner :public QObject {
    Q_OBJECT

public:
    explicit Runner(FolderManager* folderManager, QObject* parent = nullptr)
        : QObject(parent), m_folderManager(folderManager) {}//, resolution(0.0), threshold(0.0), ssim(0.0), iteration_num(0) {}

public slots:
    void start(const QStringList& inputs);

signals:
    void finished(const QString& result);   // 任务完成信号

private:
    QString performTask(); // 实际执行的任务
    double resolution;
    double threshold;
    double ssim;
    int iteration_num;

    FolderManager* m_folderManager;
};

#endif // RUNNER_H
