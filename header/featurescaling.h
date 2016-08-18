#ifndef FEATURESCALING_H
#define FEATURESCALING_H

#include <opencv2/core/core.hpp>

class FeatureScaling
{
private:
    cv::Mat colMin, colMax;

public:
    FeatureScaling(){}
    FeatureScaling(cv::Mat & trainingFeatures)
    {
        cv::reduce(trainingFeatures, colMin, 0, CV_REDUCE_MIN);
        cv::reduce(trainingFeatures, colMax, 0, CV_REDUCE_MAX);
        scale(trainingFeatures);
    }
    FeatureScaling(const cv::Mat & trainingFeatures)
    {
        cv::reduce(trainingFeatures, colMin, 0, CV_REDUCE_MIN);
        cv::reduce(trainingFeatures, colMax, 0, CV_REDUCE_MAX);
    }

    void scale(cv::Mat & features) const
    {
        for (int i = 0; i < features.cols; i++)
        {
            features.col(i) -= colMin.at<float>(i);
            features.col(i) /= colMax.at<float>(i) - colMin.at<float>(i);
        }
    }

};


#endif // FEATURESCALING_H
