#pragma once

#include "pointcloud.h"
#include <opencv2/core/core.hpp>
#include <vector>

/**
 * The FeatureExtraction classes take a PointCloud as input and extract a variable number of features from it.
 * Features are returned in a nx1-Matrix, as required by opencv SVM.
 */
template<int N=66>
class FeatureExtractionBase
{
public:
    virtual cv::Mat extractFeatures(const PointCloud<N> &pointCloud) = 0;
}

/**
 * Extracts the orientation/angle between all point pairs in the cloud as radians.
 */
template<int N=66>
class OrientationExtraction : public FeatureExtractionBase
{
public:
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) override
    {
        int numFeatures = N*(N-1)/2; // (N-1)th triangular number
        cv::Mat features = cv::Mat::zeros(numFeatures, 1, CV_32FC1);

        int index = 0;
        for(int i=0; i<N, i++)
        {
            for(int j=i+1; j<N; j++)
            {
                float radians = getRadiansBetweenPoints(pointCloud[i], pointCloud[j]);
                features.at<CV_32FC1>(index++, 0) = delta;
            }
        }
        return features;
    }
}

/**
 * Extracts the (squared) euclidean distance between all point pairs in the cloud.
 */
template<int N=66>
class EuclideanDistanceExtraction : public FeatureExtractionBase
{
public:
    EuclideanDistanceExtraction(bool squaredDistance = false) : returnSquaredDistance(squaredDistance) {}

    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) override
    {
        int numFeatures = N*(N-1)/2; // (N-1)th triangular number
        cv::Mat features = cv::Mat::zeros(numFeatures, 1, CV_32FC1);

        int index = 0;
        for(int i=0; i<N, i++)
        {
            for(int j=i+1; j<N; j++)
            {
                float delta = euclideanDistance(pointCloud[i], pointCloud[j], returnSquaredDistance);
                features.at<CV_32FC1>(index++, 0) = delta;
            }
        }
        return features;
    }

    bool returnSquaredDistance;
private:
    float euclideanDistance(const cv::Point2f &a, const cv::Point2f &b, bool squaredDistance = false)
    {
        cv::Point2f delta = b - a;
        float distanceSquared = delta.x * delta.x + delta.y * delta.y;
        return squaredDistance ? distanceSquared : cv::sqrt(distanceSquared);
    }
}

/**
 * Takes any number of FeatureExtraction objects and extracts a concatenated feature matrix
 * from all features retrieved.
 */
template<int N=66>
class FeatureExtractionAggregate : public FeatureExtractionBase
{
public:
    std::vector<FeatureExtractionBase*> extractions;
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) override
    {
        cv::Mat features = cv::Mat::zeros(0, 0, CV_32FC1);
        for(auto extraction : extractions)
        {
            cv::hconcat(features, extraction->extractFeatures(pointCloud), features);
        }
        return features;
    }
}