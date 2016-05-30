#ifndef QDATALOADER_H
#define QDATALOADER_H

#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include "actionunit.h"
#include <QtQml>

class QActionUnit;

class QDataLoader: public QObject
{
    Q_OBJECT
public:
    QDataLoader(QObject* parent = nullptr);
    Q_INVOKABLE QVariantList loadLandmarks(const QString & filename) const;
    Q_INVOKABLE QActionUnit* loadActionUnit(const QString & filename);
    //Load landmarks from different persons by iterating through a directory
    Q_INVOKABLE QVariantList loadLandmarksIterateDir(const QString & directory) const;
    //Load action unit from different persons by iterating through a directory
    Q_INVOKABLE QVariantList loadActionUnitIterateDir(const QString & directory) ;
};

class QActionUnit: public QObject{
    Q_OBJECT
    Q_PROPERTY(int length READ length)
public:
    QActionUnit(ActionUnit && unit, QObject* parent=nullptr);
    QActionUnit(QActionUnit & other) = default;
    Q_INVOKABLE QVariantMap actionIntensity(int frame) const;
    int length() const;
private:
    ActionUnit unit;
};

#endif // QDATALOADER_H
