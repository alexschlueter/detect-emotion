#include "qlandmark.h"
#include "normalization.h"
#include <QtCore/QPointF>


QVariantList QLandmark::points(){
    return pointToVarList<66>(_points.points());
}


QLandmark* QLandmark::normalized(){
    auto points = centerPointCloud(_points);
    points = rotatePointCloudOnEyeAndForehead(points);
    //points = rotatePointCloudOnForeHead(points);
    points = scalePointCloud2(points,1);
    points = points.scale(600);
    return new QLandmark(std::move(points),this);
}
