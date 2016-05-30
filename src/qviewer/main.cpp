#include <QtWidgets/QApplication>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "qdataloader.h"

int main(int argc, char** argv){
    QApplication app(argc, argv);

    qmlRegisterUncreatableType<QActionUnit,1>("QActionUnit",1,0,"QActionUnit", "Need a cv::Mat/ActionUnit for constructing");

    QQmlApplicationEngine engine;
    QDataLoader loader;
    engine.rootContext()->setContextProperty("loader",&loader);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
