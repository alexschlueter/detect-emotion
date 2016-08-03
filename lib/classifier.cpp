#include "classifier.h"
#include <opencv2/ml/ml.hpp>


ConfusionMatrix computeConfusionMatrixFromTestSet(const Classifier & c,const cv::Mat &testset, const cv::Mat &truthset){
    ConfusionMatrix cm;
    cm[0] = {0,0};
    cm[1] = {0,0};
    assert(testset.rows == truthset.rows);
    cv::Mat truthset_s8;
    truthset.convertTo(truthset_s8,CV_8SC1);

    for (int i = 0; i < testset.rows; i++){
        auto res = c.classify(testset.row(i));
        if (res == failure) {
            cm[0] = {0,0};
            cm[1] = {0,0};
            return cm;
            // TODO: Error treatment
        }
        auto truth = truthset_s8.at<int8_t>(i, 0);
        cm[truth==action? 0 : 1][res==action? 0 : 1]++;
    }
    return cm;
}

