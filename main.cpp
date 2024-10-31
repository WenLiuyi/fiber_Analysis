#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Runner.h"
#include "FolderManager.h"
#include "File.h"

#pragma execution_character_set("utf-8")

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    qmlRegisterType<File>("MyFile", 1, 0, "File");

    Runner runner;  // 创建 Runner 实例
    engine.rootContext()->setContextProperty("runner", &runner);  // 注册到 QML

    FolderManager* folderManager = new FolderManager();
    engine.rootContext()->setContextProperty("folderManager", folderManager);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/qtquickapplication1/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
