#ifndef RUNNER_H
#define RUNNER_H

#include<QObject>
#include<QDebug>
#include <QStringList>

class Runner:public QObject{
    Q_OBJECT

public:
    explicit Runner(QObject* parent = nullptr) : QObject(parent), resolution(0.0), threshold(0.0), ssim(0.0), iteration_num(0) {}
    Q_INVOKABLE void run(const QStringList& inputs) {
        if (inputs.size() < 4) {
            qWarning() << "Insufficient input values.";
            return;
        }

        // 将输入数据转为浮点数并存储
        bool ok;
        resolution = inputs[0].toDouble(&ok);
        if (!ok) {
            qWarning() << "Invalid resolution value:" << inputs[0];
        }

        threshold = inputs[1].toDouble(&ok);
        if (!ok) {
            qWarning() << "Invalid threshold value:" << inputs[1];
        }

        ssim = inputs[2].toDouble(&ok);
        if (!ok) {
            qWarning() << "Invalid ssim value:" << inputs[2];
        }

        iteration_num = inputs[3].toInt(&ok);
        if (!ok) {
            qWarning() << "Invalid iternum value:" << inputs[3];
        }

        // 输出属性值
        qDebug() << "Resolution:" << resolution;
        qDebug() << "Threshold:" << threshold;
        qDebug() << "SSIM:" << ssim;
        qDebug() << "Iteration Number:" << iteration_num;
    }



public slots:
    void run();     // 执行runner.cpp中的逻辑

signals:
    void finished(const QString &result);   // 任务完成信号

private:
    QString performTask(); // 实际执行的任务
    double resolution;
    double threshold;
    double ssim;
    int iteration_num;
};

#endif // RUNNER_H
