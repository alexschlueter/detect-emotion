#ifndef NORMALIZATION_H
#define NORMALIZATION_H

#include <vector>
#include <array>
#include <opencv2/core/core.hpp>
#include <cmath>
#include "pointcloud.h"

const int INDEX_BROW_LEFT = 17;
const int INDEX_BROW_RIGHT = 26;
const int INDEX_FOREHEAD_LEFT = 0;
const int INDEX_FOREHEAD_RIGHT = 16;
const int INDEX_EYE_LEFT = 36;
const int INDEX_EYE_RIGHT = 45;
const int INDEX_MOUTH_LEFT = 48;
const int INDEX_MOUTH_RIGHT = 54;

template <typename T>
cv::Point2f operator/(const cv::Point2f&a, T factor){
    return cv::Point2f(a.x / factor, a.y / factor);
}


template<int N=66>
PointCloud<N> centerPointCloud(const PointCloud<N> & points) {
    return points.translate(points.midPoint());
}

template<int N=66>
void centerPointCloudVector(std::vector<PointCloud<N>> &landmarks)
{
    for(auto &pointCloud : landmarks)
    {
        pointCloud = centerPointCloud<N>(pointCloud);
    }
}

template<int N=66>
float getPointCloudSquaredSum(const PointCloud<N> & points)
{
    float squaredSum = 0;
    for(auto point : points.points())
    {
        squaredSum += (point.x*point.x + point.y*point.y);
    }
    return squaredSum;
}

template<int N=66>
PointCloud<N> scalePointCloud(const PointCloud<N> & points, float resolution)
{
    float sqrtSquaredSum = cv::sqrt(getPointCloudSquaredSum<N>(points));

    return points.scale(resolution / sqrtSquaredSum );
}

template<int N=66>
PointCloud<N> scalePointCloud2(const PointCloud<N> & points, float resolution = 1.0f)
{
    float ydist = points.greatestYDistance();
    return points.scale(resolution / ydist );
}

template<int N=66>
void scalePointCloudVector(std::vector<PointCloud<N>> &pointCloudVector, float resolution = 1)
{
    for(auto &pointCloud : pointCloudVector)
    {
        pointCloud = scalePointCloud<N>(pointCloud, resolution);
    }
}


/** Normalizes the point cloud rotation by getting the average angle of the straight line between
*   eye and forehead landmarks and rotating the point cloud against it.
**/
template<int N=66>
PointCloud<N> rotatePointCloud(const PointCloud<N> &pointCloud)
{
    cv::Point2f eyeLine = pointCloud[INDEX_EYE_RIGHT] - pointCloud[INDEX_EYE_LEFT];
    cv::Point2f foreheadLine = pointCloud[INDEX_FOREHEAD_RIGHT] - pointCloud[INDEX_FOREHEAD_LEFT];
    cv::Point2f xAxis(1, 0);
    float rotationNeeded = (getRadiansBetweenPoints(xAxis, eyeLine) + getRadiansBetweenPoints(xAxis, foreheadLine)) / 2.f;

    return pointCloud.rotate(rotationNeeded);
}

template<int N=66>
void rotatePointCloudVector(std::vector<PointCloud<N>> &landmarks)
{
    for(auto &pointCloud : landmarks)
    {
        pointCloud = rotatePointCloud<N>(pointCloud);
    }
}

template<int N=66>
void standardizePointCloudVector(std::vector<PointCloud<N>> &pointCloudVector, float resolution = 1)
{
    centerPointCloudVector<N>(pointCloudVector);
    scalePointCloudVector<N>(pointCloudVector, resolution);
    rotatePointCloudVector<N>(pointCloudVector);
}

#endif
