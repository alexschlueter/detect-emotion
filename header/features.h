#pragma once

#include "pointcloud.h"
#include <opencv2/core/core.hpp>
#include <vector>
#include <math.h>
#include <memory>

const float PI = std::atan(1.0)*4;

/**
 * The FeatureExtraction classes take a PointCloud as input and extract a variable number of features from it.
 * Features are returned in a nx1-Matrix, as required by opencv SVM.
 */
template<int N=66>
class FeatureExtractionBase
{
public:
    virtual cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const = 0;
};

/**
 * Extracts the orientation/angle between all point pairs in the cloud as radians.
 * Returns a 1xM matrix.
 */
template<int N=66>
class OrientationExtraction : public FeatureExtractionBase<N>
{
public:
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
        int numFeatures = N*(N-1)/2; // (N-1)th triangular number
        cv::Mat features = cv::Mat::zeros(1, numFeatures, CV_32FC1);

        int index = 0;
        for(int i=0; i<N; i++)
        {
            for(int j=i+1; j<N; j++)
            {
                float radians = getRadiansBetweenPoints(pointCloud[i], pointCloud[j]) + PI;
                float normalizedOrientation = radians / (2*PI);
                features.at<float>(0, index++) = normalizedOrientation;
            }
        }
        return features;
    }
};

/**
 * Extracts the (squared) euclidean distance between all point pairs in the cloud.
 * Returns a 1xM matrix.
 */
template<int N=66>
class EuclideanDistanceExtraction : public FeatureExtractionBase<N>
{
public:
    EuclideanDistanceExtraction(bool squaredDistance = false) : returnSquaredDistance(squaredDistance) {}

    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
        int numFeatures = N*(N-1)/2; // (N-1)th triangular number
        cv::Mat features = cv::Mat::zeros(1, numFeatures, CV_32FC1);

        int index = 0;
        for(int i=0; i<N; i++)
        {
            for(int j=i+1; j<N; j++)
            {
                float delta = euclideanDistance(pointCloud[i], pointCloud[j], returnSquaredDistance);
                features.at<float>(0, index++) = delta;
            }
        }
        return features;
    }

    bool returnSquaredDistance;
private:
    float euclideanDistance(const cv::Point2f &a, const cv::Point2f &b, bool squaredDistance = false) const
    {
        cv::Point2f delta = b - a;
        float distanceSquared = delta.x * delta.x + delta.y * delta.y;
        return squaredDistance ? distanceSquared : cv::sqrt(distanceSquared);
    }
};

/**
 * Takes any number of FeatureExtraction objects and extracts a concatenated feature matrix
 * from all features retrieved. The matrix 1xM, where M is the total number
 * of features extracted.
 */
template<int N=66>
class FeatureExtractionAggregate : public FeatureExtractionBase<N>
{
public:
    std::vector<std::shared_ptr<FeatureExtractionBase<N>>> extractions;
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
        cv::Mat features = cv::Mat::zeros(0, 0, CV_32FC1);

        if(extractions.size() == 0)
            return features;

        features = extractions[0]->extractFeatures(pointCloud);
        for(int i=1; i<extractions.size(); i++)
        {
            cv::hconcat(features, extractions[i]->extractFeatures(pointCloud), features);
        }
        return features;
    }
};

template<int N=66>
cv::Mat extractFeaturesFromData(const std::vector<PointCloud<N>> &data, const FeatureExtractionBase<N> *extractor)
{
    if(data.size() == 0)
        return cv::Mat();

    cv::Mat result = extractor->extractFeatures(data[0]);
    for(int i=1; i<data.size(); i++)
    {
        cv::vconcat(result, extractor->extractFeatures(data[i]));
    }
    return result;
}