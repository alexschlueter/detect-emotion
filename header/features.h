#pragma once

#include "pointcloud.h"
#include <opencv2/core/core.hpp>
#include <vector>
#include <math.h>
#include <memory>

const float PI = 3.141592653589793;

/**
 * The FeatureExtraction classes take a PointCloud as input and extract a variable number of features from it.
 * Features are returned in a nx1-Matrix, as required by opencv SVM.
 */
template<int N=66>
class FeatureExtractionBase
{
public:
    virtual cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const = 0;
    virtual unsigned int getNumFeatures() const = 0;
};


/**
 * Simply returns the x and y coordinates as features.
 */
template<int N=66>
class XYFeatureExtraction : public FeatureExtractionBase<N>
{
public:
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
        int numFeatures = getNumFeatures();
        cv::Mat features = cv::Mat::zeros(1, numFeatures, CV_32FC1);

        for(int i=0; i<N-1; i+=2)
        {
            features.at<float>(0, i) = pointCloud[i].x;
            features.at<float>(0, i + 1) = pointCloud[i].y;
        }
        return features;
    }
    unsigned int getNumFeatures() const override
    {
        return 2 * N;
    }
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
        int numFeatures = getNumFeatures();
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
    unsigned int getNumFeatures() const override
    {
        // (N-1)th triangular number
        return N * (N-1)/2;
    }
};

template<int N=66>
class EuclideanDistanceExtractionBase : public FeatureExtractionBase<N>
{
public:
    EuclideanDistanceExtractionBase(bool squaredDistance = false) : returnSquaredDistance(squaredDistance) {}

    virtual cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const = 0;
    virtual unsigned int getNumFeatures() const = 0;

    bool returnSquaredDistance;
protected:
    float euclideanDistance(const cv::Point2f &a, const cv::Point2f &b, bool squaredDistance = false) const
    {
        cv::Point2f delta = b - a;
        float distanceSquared = delta.x * delta.x + delta.y * delta.y;
        return squaredDistance ? distanceSquared : cv::sqrt(distanceSquared);
    }
};

/**
 * Extracts the (squared) euclidean distance between all point pairs in the cloud.
 * Returns a 1xM matrix.
 */
template<int N=66>
class EuclideanDistanceExtraction : public EuclideanDistanceExtractionBase<N>
{
public:
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
        int numFeatures = getNumFeatures();
        cv::Mat features = cv::Mat::zeros(1, numFeatures, CV_32FC1);

        int index = 0;
        for(int i=0; i<N; i++)
        {
            for(int j=i+1; j<N; j++)
            {
                float delta = this->euclideanDistance(pointCloud[i], pointCloud[j], this->returnSquaredDistance);
                features.at<float>(0, index++) = delta;
            }
        }
        return features;
    }
    unsigned int getNumFeatures() const override
    {
        // the n-th triangular number
        return N*(N-1)/2;
    }
};

/**
 * Returns the euclidean distance to the point cloud center as features.
 */
template<int N=66>
class CenterDistanceExtraction : public EuclideanDistanceExtractionBase<N>
{
public:
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
        int numFeatures = getNumFeatures();
        cv::Mat features = cv::Mat::zeros(1, numFeatures, CV_32FC1);

        cv::Point2f midPoint = pointCloud.midPoint();
        for(int i=0; i<N; i++)
        {
            float delta = this->euclideanDistance(pointCloud[i], midPoint, this->returnSquaredDistance);
            features.at<float>(0, i) = delta;
        }
        return features;
    }
    unsigned int getNumFeatures() const override
    {
        // the n-th triangular number
        return N;
    }
};

/**
 * Returns the Orientation in relation to the pointCloud center as features.
 */
template<int N=66>
class CenterOrientationExtraction : public FeatureExtractionBase<N>
{
public:
    cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
        int numFeatures = getNumFeatures();
        cv::Mat features = cv::Mat::zeros(1, numFeatures, CV_32FC1);

        cv::Point2f midPoint = pointCloud.midPoint();
        for(int i=0; i<N; i++)
        {
            float radians = getRadiansBetweenPoints(pointCloud[i], midPoint) + PI;
            float normalizedOrientation = radians / (2*PI);
            features.at<float>(0, i) = normalizedOrientation;
        }
        return features;
    }
    unsigned int getNumFeatures() const override
    {
        // the n-th triangular number
        return N;
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
            int numCols = features.cols;
            cv::hconcat(features, extractions[i]->extractFeatures(pointCloud), features);
            assert(features.cols == numCols + extractions[i]->getNumFeatures());
        }
        return features;
    }
    unsigned int getNumFeatures() const override
    {
        unsigned int result = 0;
        for(auto extractor : extractions)
        {
            result += extractor->getNumFeatures();
        }
        return result;
    }
};

#include "pca.h"
template<int N=66>
class SimpleNormalizeFeatureExtraction: public FeatureExtractionBase<N>
{
public:
    SimpleNormalizeFeatureExtraction(int pca_dimension=2*N): _pca_dimension(pca_dimension){}
    virtual cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const{
        if (_pca != nullptr){
            return _pca->project(pointCloud,_pca_dimension);
        }else { // No PCA -> just normalization
            return standardNormalization(pointCloud).asMat();
        }
    }
    virtual unsigned int getNumFeatures() const {
       return _pca_dimension;
    }

    void compute_pca(const std::vector<PointCloud<N>> & points ) {
        _pca = std::unique_ptr<PCA_Result<N>>(new PCA_Result<N>(computePCA<N>(points)));
    }

    void set_pca( std::unique_ptr<PCA_Result<N>> &&pca) {_pca = std::move(pca);}

private:
    int _pca_dimension;
    std::unique_ptr<PCA_Result<N>> _pca;
};

/**
 * Helper function to extract features from a vector of point clouds.
 */
template<int N=66>
cv::Mat extractFeaturesFromData(const std::vector<PointCloud<N>> &data, const FeatureExtractionBase<N> *extractor)
{
    if(data.size() == 0)
        return cv::Mat();

    // calculate required matrix size
    unsigned int numRows = data.size();
    unsigned int numCols = extractor->getNumFeatures();
    cv::Mat result(numRows, numCols, CV_32FC1);

    // extract features for each row
    for(int i=0; i<numRows; i++)
    {
        extractor->extractFeatures(data[i]).copyTo(result.row(i));
    }
    return result;
}
