#ifndef NORMALIZATION_H
#define NORMALIZATION_H

#include <vector>
#include <array>
#include <opencv2/core/core.hpp>
#include <cmath>

const int INDEX_BROW_LEFT = 17;
const int INDEX_BROW_RIGHT = 26;
const int INDEX_FOREHEAD_LEFT = 0;
const int INDEX_FOREHEAD_RIGHT = 16;
const int INDEX_EYE_LEFT = 36;
const int INDEX_EYE_RIGHT = 45;
const int INDEX_MOUTH_LEFT = 48;
const int INDEX_MOUTH_RIGHT = 54;

template<int N=66>
cv::Point2f getPointCloudCenter(const std::array<cv::Point2f, N> &pointCloud)
{
    cv::Point2f center(0, 0);
    for(auto p : pointCloud)
    {
        center += p;
    }
    return center / float(pointCloud.size());
}

template<int N=66>
void centerPointCloud(std::array<cv::Point2f, N> &pointCloud)
{
    cv::Point2f center(getPointCloudCenter<N>(pointCloud));
    for(auto &p : pointCloud)
    {
        p -= center;
    }
}

template<int N=66>
void centerPointCloudVector(std::vector<std::array<cv::Point2f, N>> &landmarks)
{
    for(auto &pointCloud : landmarks)
    {
        centerPointCloud<N>(pointCloud);
    }
}

template<int N=66>
float getPointCloudSquaredSum(std::array<cv::Point2f, N> &pointCloud)
{
    float squaredSum = 0;
    for(auto point : pointCloud)
    {
        squaredSum += (point.x*point.x + point.y*point.y);
    }
    return squaredSum;
}

template<int N=66>
void scalePointCloud(std::array<cv::Point2f, N> &pointCloud, float resolution = 1)
{
    float sqrtSquaredSum = cv::sqrt(getPointCloudSquaredSum<N>(pointCloud));

    for(auto &point : pointCloud)
    {
        point = point / sqrtSquaredSum * resolution;
    }
}

template<int N=66>
void scalePointCloudVector(std::vector<std::array<cv::Point2f, N>> &pointCloudVector, float resolution = 1)
{
    for(auto &pointCloud : pointCloudVector)
    {
        scalePointCloud<N>(pointCloud, resolution);
    }
}


/** BELOW IS WIP */
float getRadiansBetweenPoints(const cv::Point2f &p1, const cv::Point2f &p2)
{
    return std::atan2(p1.y, p1.x) - std::atan2(p2.y, p2.x);
}

cv::Point2f rotatePoint(const cv::Point2f &point, float radians)
{
    cv::Point2f result;
    result.x = point.x * std::cos(radians) - point.y * std::sin(radians);
    result.y = point.x * std::sin(radians) + point.y * std::cos(radians);
    return result;
}

/** Normalizes the point cloud rotation by getting the average angle of the straight line between
*   eye and forehead landmarks and rotating the point cloud against it.
**/
template<int N=66>
void rotatePointCloud(std::array<cv::Point2f, N> &pointCloud)
{
    cv::Point2f eyeLine = pointCloud[INDEX_EYE_RIGHT] - pointCloud[INDEX_EYE_LEFT];
    cv::Point2f foreheadLine = pointCloud[INDEX_FOREHEAD_RIGHT] - pointCloud[INDEX_FOREHEAD_LEFT];
    cv::Point2f xAxis(1, 0);
    float rotationNeeded = (getRadiansBetweenPoints(xAxis, eyeLine) + getRadiansBetweenPoints(xAxis, foreheadLine)) / 2.f;

    for(auto &point : pointCloud)
    {
        point = rotatePoint(point, rotationNeeded);
    }
}

template<int N=66>
void rotatePointCloudVector(std::vector<std::array<cv::Point2f, N>> &landmarks)
{
    for(auto &pointCloud : landmarks)
    {
        rotatePointCloud<N>(pointCloud);
    }
}

template<int N=66>
void standardizePointCloudVector(std::vector<std::array<cv::Point2f, N>> &pointCloudVector, float resolution = 1)
{
    centerPointCloudVector<N>(pointCloudVector);
    scalePointCloudVector<N>(pointCloudVector, resolution);
    rotatePointCloudVector<N>(pointCloudVector);
}

#endif