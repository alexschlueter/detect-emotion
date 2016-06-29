#include "classifier.h"
#include <opencv2/ml/ml.hpp>
class SVMClassifier: public Classifier{
public:
    explicit SVMClassifier(std::unique_ptr<CvSVM> && svm): _svm(std::move(svm)){}

    virtual int classify(const cv::Mat & feature) const{
        return _svm->predict(feature,true) < 0? 0 : 1;
    }
    virtual bool serialize(const std::string & filename) const {
        _svm->save(filename.c_str());
        return true;
    }
private:
    std::unique_ptr<CvSVM> _svm;
};


template <typename T>
inline cv::Mat constructTrainMatT(const FeatureList & trainingsset){
    if (trainingsset.empty()) return cv::Mat();
    auto cols = trainingsset[0].cols;
    cv::Mat trainMat(trainingsset.size(),cols,CV_32FC1);
    for (int i=0; i< trainingsset.size(); i++){
        if (trainingsset[i].cols != cols || trainingsset[i].rows == 0) return cv::Mat();
        float* dst_ptr = trainMat.ptr<float>(i);
        if (cv::DataType<T>::type != trainingsset[i].type()) return cv::Mat();
        const T* src_ptr = trainingsset[i].ptr<T>(0);
        for (int j=0; j<cols; j++){
            dst_ptr[j] =  static_cast<float>(src_ptr[j]);
        }
    }
    return trainMat;
}

inline cv::Mat constructTrainMat(const FeatureList &trainingsset){
    if (trainingsset.empty()) return cv::Mat();
    switch(trainingsset[0].type()){
    case CV_8UC1: return constructTrainMatT<uint8_t>(trainingsset);
    case CV_8SC1: return constructTrainMatT<int8_t>(trainingsset);
    case CV_16UC1: return constructTrainMatT<uint16_t>(trainingsset);
    case CV_16SC1: return constructTrainMatT<int16_t>(trainingsset);
    case CV_32SC1: return constructTrainMatT<int32_t>(trainingsset);
    case CV_32FC1: return constructTrainMatT<float>(trainingsset);
    case CV_64FC1: return constructTrainMatT<double>(trainingsset);
    default: return cv::Mat();
    }
}

cv::Mat constructTruthMap(const TruthList & list){
    if (list.empty()) return cv::Mat();
    cv::Mat truth(list.size(),1,CV_32FC1);
    for (int i=0; i< list.size(); i++){
        truth.at<float>(i,0) = static_cast<float>(list[i]);
    }
    return truth;
}

SVMConstructor::SVMConstructor(CvSVMParams params):_params(params){

}

SVMConstructor::SVMConstructor(){
    _params.svm_type = CvSVM::C_SVC;
    _params.kernel_type = CvSVM::LINEAR;
    _params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER,1000,1e-6);
}

std::unique_ptr<Classifier> SVMConstructor::train(const FeatureList & trainingsset, const TruthList& truthset) const{
    auto res = std::unique_ptr<CvSVM>(new CvSVM());
    assert(trainingsset.size() == truthset.size());
    cv::Mat trainMat = constructTrainMat(trainingsset);
    if(trainMat.empty()) return nullptr;
    cv::Mat traintruth = constructTruthMap(truthset);
    if(traintruth.empty()) return nullptr;
    res->train(trainMat, traintruth, cv::Mat(), cv::Mat(), _params);
    return std::unique_ptr<Classifier>(new SVMClassifier(std::move(res)));
}

std::unique_ptr<Classifier> SVMConstructor::deserialize(const std::string & filename) const{
    CvSVM * res = new CvSVM();
    res->load(filename.c_str());
    return std::unique_ptr<Classifier>(new SVMClassifier(std::unique_ptr<CvSVM>(res)));
}

ConfusionMatrix computeConfusionMatrixFromTestSet(const Classifier & c,const FeatureList &testset, const TruthList &truthset){
    ConfusionMatrix cm;
    cm[0] = {0,0}; cm[1] = {0,0};
    assert(testset.size() == truthset.size());
    for (int i=0; i< testset.size(); i++){
        auto res = c.classify(testset[i]);
        if (res < 0) {
            cm[0] = {0,0}; cm[1] = {0,0};
            return cm;
            // TODO: Error treatment
        }
        assert(res < 2);
        auto truth = truthset[i];
        assert(truth >= 0 && truth < 2);
        cm[truth][res]++;
    }
    return cm;
}

