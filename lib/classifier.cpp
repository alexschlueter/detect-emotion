#include "classifier.h"
#include <opencv2/ml/ml.hpp>

class SVMClassifier: public Classifier{
public:
    explicit SVMClassifier(std::unique_ptr<CvSVM> && svm): _svm(std::move(svm)){}

    virtual int classify(const cv::Mat & feature) const{
        return _svm->predict(feature, true) < 0 ? 0 : 1;
    }
    virtual bool serialize(const std::string & filename) const {
        _svm->save(filename.c_str());
        return true;
    }
private:
    std::unique_ptr<CvSVM> _svm;
};

SVMConstructor::SVMConstructor(CvSVMParams params):_params(params){

}

SVMConstructor::SVMConstructor(){
    _params.svm_type = CvSVM::C_SVC;
    _params.kernel_type = CvSVM::LINEAR;
    _params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER,1000,1e-6);
}

std::unique_ptr<Classifier> SVMConstructor::train(const cv::Mat & trainingsset, const cv::Mat& truthset) const{
    auto res = std::unique_ptr<CvSVM>(new CvSVM());
    assert(trainingsset.rows == truthset.rows);
    cv::Mat truthAsFloat;
    truthset.convertTo(truthAsFloat, CV_32F);
    res->train(trainingsset, truthAsFloat, cv::Mat(), cv::Mat(), _params);
    return std::unique_ptr<Classifier>(new SVMClassifier(std::move(res)));
}

std::unique_ptr<Classifier> SVMConstructor::deserialize(const std::string & filename) const{
    CvSVM * res = new CvSVM();
    res->load(filename.c_str());
    return std::unique_ptr<Classifier>(new SVMClassifier(std::unique_ptr<CvSVM>(res)));
}

ConfusionMatrix computeConfusionMatrixFromTestSet(const Classifier & c,const cv::Mat &testset, const cv::Mat &truthset){
    ConfusionMatrix cm;
    cm[0] = {0,0};
    cm[1] = {0,0};
    assert(testset.rows == truthset.rows);

    for (int i = 0; i < testset.rows; i++){
        auto res = c.classify(testset.row(i));
        if (res < 0) {
            cm[0] = {0,0};
            cm[1] = {0,0};
            return cm;
            // TODO: Error treatment
        }
        assert(res < 2);
        auto truth = truthset.at<uint8_t>(i, 0);
        assert(truth >= 0 && truth < 2);
        cm[truth][res]++;
    }
    return cm;
}

