#include <QtWidgets/QApplication>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "qdataloader.h"
#include "qlandmark.h"

int main(int argc, char** argv){
    QApplication app(argc, argv);

    qmlRegisterUncreatableType<QActionUnit,1>("QActionUnit",1,0,"QActionUnit", "Need a cv::Mat/ActionUnit for constructing");
    qmlRegisterUncreatableType<QLandmark,1>("QLandmark",1,0,"QLandmark", "");

    QQmlApplicationEngine engine;
    QDataLoader loader;
    engine.rootContext()->setContextProperty("loader",&loader);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
