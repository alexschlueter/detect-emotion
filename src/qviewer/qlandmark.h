#ifndef QLANDMARK_H
#define QLANDMARK_H
#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <opencv2/core/core.hpp>
#include "normalization.h"
#include "pointcloud.h"

/**
 * Class for using Landmarks in QML
 */
class QLandmark: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList points  READ points NOTIFY pointsChanged)
public:
    QLandmark(const PointCloud<66> & points, QObject* parent=nullptr): QObject(parent),_points(points){}
    QLandmark(PointCloud<66> && points, QObject* parent=nullptr): QObject(parent),_points(points){}
    QLandmark(const QLandmark & l ): _points(l._points){}
    QLandmark(QLandmark && l ): _points(std::move(l._points)){}
    QLandmark(QObject* parent = nullptr): QObject(parent){}

    void operator=(const QLandmark & other){
       _points = other._points;
       emit pointsChanged();
    }

    void operator=(QLandmark && other){
       _points = std::move(other._points);
       emit pointsChanged();
    }

    QVariantList points();
    // Normalize the points
    Q_INVOKABLE QVariantList normalized();
signals:
    void pointsChanged();
private:
     PointCloud<66> _points;
};

Q_DECLARE_METATYPE(QLandmark)
Q_DECLARE_METATYPE(QLandmark*)

#endif // QLANDMARK_H
