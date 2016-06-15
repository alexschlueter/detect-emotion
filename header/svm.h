#pragma once
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

using namespace cv;

class SVM
{
public:
    SVM()
    {
        svm = ml::SVM::create();
        svm->setType(ml::SVM::C_SVC);
        svm->setKernel(ml::SVM::LINEAR);
        svm->setGamma(3);
    }

    void train(const cv::Mat &samples, const cv::Mat &labels, bool dataAsRow = true)
    {
        assert((dataAsRow && samples.rows == labels.rows) || (!dataAsRow || samples.cols == labels.cols));
        svm->train(samples , dataAsRow ? ml::ROW_SAMPLE : ml::COL_SAMPLE, labels);
    }

    float predict(const cv::Mat &sample)
    {
        return svm->predict(sample);
    }
private:
    cv::Ptr<ml::SVM> svm;
};