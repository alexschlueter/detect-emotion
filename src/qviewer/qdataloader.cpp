#include "qdataloader.h"
#include "reader.h"
#include "actionunit.h"
#include <QMap>
#include <QString>
#include <QVariant>
#include <QPointF>
#include "qlandmark.h"

QDataLoader::QDataLoader(QObject* parent): QObject(parent)
{

}

QActionUnit::QActionUnit(ActionUnit && _unit,QObject* parent): QObject(parent), unit(_unit){}

QVariantList QDataLoader::loadLandmarksIterateDir(const QString &directory) {
    auto entrylist = QDir(directory).entryInfoList(QStringList()<<"*.bin",QDir::Files);
    QVariantList result;
    for (auto entry: entrylist){
        result.append(QVariant::fromValue(this->loadLandmarks(entry.absoluteFilePath())));
    }
    return result;
}

QVariantList QDataLoader::loadLandmarks(const QString & filename) {
    QVariantList result;
    auto res = readBinaryFile(filename.toStdString());
    for (auto array: res){
        result.append(QVariant::fromValue(new QLandmark(array, this)));
    }
    return result;
}


QVariantList QDataLoader::loadActionUnitIterateDir(const QString &directory) {
    auto persondirs = QDir(directory).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    QVariantList result;
    for (auto entry: persondirs){
        auto filename = QStringLiteral("%1/%2_AUs.txt").arg(entry.absoluteFilePath()).arg(entry.baseName());
        result.append(QVariant::fromValue(this->loadActionUnit(filename)));
    }
    return result;
}

QActionUnit* QDataLoader::loadActionUnit(const QString &filename){
    return new QActionUnit(readActionUnitFromFile(filename.toStdString()),this);
}

QVariantMap QActionUnit::actionIntensity(int frame) const{
    //QHash<QString, int> result;
    QVariantMap result;
    auto names = unit.getActionsAsName();
    auto intens = unit.getActionsIntensity(frame);
    assert(names.size() == intens.size());
    for (int i=0; i< names.size(); i++){
        result.insert(QString::fromStdString(names[i]),QVariant::fromValue(intens[i]));
    }
    return result;
}

int QActionUnit::length() const{
    return unit.size();
}

