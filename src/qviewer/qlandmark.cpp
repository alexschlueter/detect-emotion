#include "qlandmark.h"
#include "normalization.h"
#include <QtCore/QPointF>

template <int N>
QVariantList pointToVarList(PointCloud<N> points){
   QVariantList res;
   for(auto p: points.points()){
       res.append(QVariant(QPointF(p.x,p.y)));
   }
   return res;
}

QVariantList QLandmark::points(){
    return pointToVarList(_points);
}


QVariantList QLandmark::normalized(){
    auto points = centerPointCloud(_points);
    //_points = rotatePointCloud(_points);
    points = scalePointCloud2(points,1);
    points = points.translate(cv::Point2f(-0.5,-0.5));
    points = points.scale(600);
    return pointToVarList(points);
}
