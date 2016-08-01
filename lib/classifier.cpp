#include "classifier.h"
#include <opencv2/ml/ml.hpp>

class SVMClassifier: public Classifier{
public:
    explicit SVMClassifier(std::unique_ptr<CvSVM> && svm): _svm(std::move(svm)){}

    virtual int classify(const cv::Mat & feature) const{
        return _svm->predict(feature, true) < 0 ? noaction : action;
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
    assert(truthset.cols == 1);
    //Transform Matrix for SVM
    cv::Mat truthAsFloat;
    truthset.convertTo(truthAsFloat, CV_32F);
    for(int i=0; i< truthset.rows; i++){
      if (truthset.as<float>(i,0)==noaction){
          truthset.as<float>(i,0)=-1;
      }else{
          truthset.as<float>(i,0) = 1;
      }
    }
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
        cm[truth==action?1:0][res==action?1:0]++;
    }
    return cm;
}

