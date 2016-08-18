#pragma once

#include "pointcloud.h"
#include <opencv2/core/core.hpp>
#include <vector>
#include <math.h>
#include <memory>
#include <fstream>

const float PI = 3.141592653589793;

/**
 * The FeatureExtraction classes take a PointCloud as input and extract a variable number of features from it.
 * Features are returned in a nx1-Matrix (data-type 1-dim float), as required by opencv SVM.
 */
template<int N=66>
class FeatureExtractionBase
{
public:
    virtual cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const = 0;
    virtual unsigned int getNumFeatures() const = 0;
    virtual std::string name() const = 0;
    virtual ~FeatureExtractionBase(){}
};

/**
 * Extract time based features using a sequence of point clouds.
 */
template <int N=66>
class TimeFeatureExtractionBase{
public:
    /**
     * Extract features based on the passed sequence of point.
     * The sequence should be the contiguous frames from the
     * source video.
     * This function needs two iterators, which corresponds
     * to the slice of the video.
     * This slice must have a length that is at least as great
     * as getNumInputFrames() returns.
     * If getNumFeatures() == 0 any length is allowed.
     * @param beg iterator which points to the first relevant element of the array
     * @param end iterator which points to the end of the relevant array part.
     *            The entry iterator points to will not be used.
     * @return cv::Mat containing one row with extracted features. cv::Mat may be empty if failures occurs (e.g. not enough frames).
     */
    virtual cv::Mat extractFeatures(typename std::vector<PointCloud<N>>::const_iterator beg, typename std::vector<PointCloud<N>>::const_iterator  end) const = 0;

    /**
     * Returns the number of features extractFeatures returns.
     * This value equals the of columns which cv::Mat contains.
     */
    virtual unsigned int getNumFeatures() const = 0;

    /**
     * Returns the number of frames/pointclouds that extractFeatures expect as input.
     * If this number is not limited, 0 should be returned.
     * @return number of frames, 0 if not limited
     */
    virtual unsigned int getNumInputFrames() const = 0;

    virtual std::string name() const = 0;

    virtual ~TimeFeatureExtractionBase(){}
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

        for(int i=0; i<N; i++)
        {
            features.at<float>(0, 2 * i) = pointCloud[i].x;
            features.at<float>(0, 2 * i + 1) = pointCloud[i].y;
        }
        return features;
    }
    unsigned int getNumFeatures() const override
    {
        return 2 * N;
    }

    std::string name() const override{
        return "XYFeatureExtraction";
    }
};

/**
 * Use the files in /masks to throw away features that are unhelpful in
 * classifying the given action unit
 */
template<int N=66>
class MaskFeatureExtraction : public FeatureExtractionBase<N>
{
private:
  std::vector<int> landmarksToKeep;

public:
  MaskFeatureExtraction(std::ifstream & maskFile)
    {
      int lm;
      while (maskFile >> lm) {
        landmarksToKeep.push_back(lm);
      }
    }
  cv::Mat extractFeatures(const PointCloud<N> &pointCloud) const override
    {
      int numFeatures = getNumFeatures();
      cv::Mat features = cv::Mat::zeros(1, numFeatures, CV_32FC1);

      for (int i = 0; i < landmarksToKeep.size(); ++i)
      {
        features.at<float>(0, 2 * i) = pointCloud[landmarksToKeep[i]].x;
        features.at<float>(0, 2 * i + 1) = pointCloud[landmarksToKeep[i]].y;
      }

      return features;
    }
  unsigned int getNumFeatures() const override
    {
      return 2 * landmarksToKeep.size();
    }
  std::string name() const override{
      std::string res = "MaskFeatureExtraction";
      for (int i: landmarksToKeep){
          res += "_"+std::to_string(i);
      }
      return res;
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
    std::string name() const{
        return "OrientationFeatureExtraction";
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
    std::string name() const{
        return "EuclideanDistanceExtraction";
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
    std::string name() const override{
        return "CenterDistanceExtraction";
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
    std::string name() const override{
        return "CenterOrientationExtraction";
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

    std::string name() const override{
        std::string res = "FeatureExtractionAggregate";
        for (auto&& sub : extractions){
            res+="_"+sub->name();
        }
        return res;
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

    std::string name() const{
        return "SimpleNormalizeFeatureExtraction";
    }
private:
    int _pca_dimension;
    std::unique_ptr<PCA_Result<N>> _pca;
};

class InterpolationFeatureExtraction: public FeatureExtractionBase<66>{
public:
    virtual cv::Mat extractFeatures(const PointCloud<66> &pointCloud) const;
     virtual unsigned int getNumFeatures() const ;
     virtual std::string name() const;
};

/**
 * Extract time based feature using numerical differentation
 * on features which a normal FeatureExtraction-Implementation produces.
 */
template <int N=66>
class SimpleTimeDifferentialExtraction: public TimeFeatureExtractionBase<N>{
public:
    SimpleTimeDifferentialExtraction(std::shared_ptr<FeatureExtractionBase<N>> base_feature): _base_feature(base_feature){  }

    virtual cv::Mat extractFeatures(typename std::vector<PointCloud<N>>::const_iterator beg, typename std::vector<PointCloud<N>>::const_iterator  end) const{
        //cv::Mat res = cv::Mat::zeros(1,getNumFeatures(),CV_32FC1);
        cv::Mat t0 = _base_feature->extractFeatures(*beg);
        beg++;
        if (beg == end) return cv::Mat();
        beg++;
        if (beg == end) return cv::Mat();
        cv::Mat t2 = _base_feature->extractFeatures(*beg);
        return (t2-t0)/2;
    }
    virtual unsigned int getNumFeatures() const{
        return _base_feature->getNumFeatures();
    }
    virtual unsigned int getNumInputFrames() const{
        return 3;
    }

    std::string name() const{
        return "SimpleTimeDifferentialExtraction_"+_base_feature->name();
    }
private:
    std::shared_ptr<FeatureExtractionBase<N>> _base_feature;
};

/**
 * Helper function to extract features from a vector of point clouds.
 */
template<int N=66>
cv::Mat extractFeaturesFromData(const std::vector<PointCloud<N>> &data, const FeatureExtractionBase<N> &extractor)
{
    if(data.size() == 0)
        return cv::Mat();

    // calculate required matrix size
    unsigned int numRows = data.size();
    unsigned int numCols = extractor.getNumFeatures();
    cv::Mat result(numRows, numCols, CV_32FC1);

    // extract features for each row
    for(int i=0; i<numRows; i++)
    {
        extractor.extractFeatures(data[i]).copyTo(result.row(i));
    }
    return result;
}

/**
 * Helper function to extract time based features from a vector of point clouds.
 */
template<int N=66>
cv::Mat extractFeaturesFromData(const std::vector<PointCloud<N>> &data, const TimeFeatureExtractionBase<N> &extractor)
{
    if(data.size() == 0)
        return cv::Mat();

    // calculate required matrix size
    unsigned int numRows = data.size();
    unsigned int numCols = extractor.getNumFeatures();
    unsigned int numSeq = extractor.getNumInputFrames();
    cv::Mat result(numRows, numCols, CV_32FC1);
    typename std::vector<PointCloud<N>>::const_iterator beg = data.begin();

    // extract features for each row
    for(int i=0; i<numRows-numSeq+1; i++)
    {
        extractor.extractFeatures(beg+i,data.end()).copyTo(result.row(i));
    }
    return result;
}
