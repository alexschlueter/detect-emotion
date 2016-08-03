#include "classifier.h"
#include <opencv2/ml/ml.hpp>

class KNearestNeighborClassifier: public Classifier{
public:
    explicit KNearestNeighborClassifier(std::unique_ptr<CvKNearest> kn,int  k): _kn(std::move(kn)),_k(k){}

    virtual ClassifierResult classify(const cv::Mat & feature) const{
        //return _svm->predict(feature) < 0 ? noaction : action;
        float res = _kn->find_nearest(feature,_k);
        if (res == 1) { return action; }
        else {return noaction;}
    }
    virtual bool serialize(const std::string & filename) const {
        //TODO: Not implemented in opencv2
        _kn->save(filename.c_str());
        return true;
    }
private:
    std::unique_ptr<CvKNearest> _kn;
    int _k;
};

KNearestNeighborsConstructor::KNearestNeighborsConstructor(int k):_k(k){}


std::unique_ptr<Classifier> KNearestNeighborsConstructor::train(const cv::Mat & trainingsset, const cv::Mat& truthset) const{
    auto res = std::unique_ptr<CvKNearest>(new CvKNearest());
    assert(trainingsset.rows == truthset.rows);
    assert(truthset.cols == 1);
    //Transform Matrix for SVM
    cv::Mat truthAsFloat;
    truthset.convertTo(truthAsFloat, CV_32F);
    for(int i=0; i< truthset.rows; i++){
      if (truthAsFloat.at<float>(i,0)==noaction){
          truthAsFloat.at<float>(i,0) = 0;
      }else{
          truthAsFloat.at<float>(i,0) = 1;
      }
    }
    res->train(trainingsset, truthAsFloat, cv::Mat(), false, _k);
    return std::unique_ptr<Classifier>(new KNearestNeighborClassifier(std::move(res),_k));
}

std::unique_ptr<Classifier> SVMConstructor::deserialize(const std::string & filename) const{
    CvKNearest * res = new CvKNearest();
    //TODO: Not implemented in opencv2
    res->load(filename.c_str());
    int k = res->get_max_k();
    return std::unique_ptr<Classifier>(new KNearestNeighborClassifier(std::unique_ptr<CvKNearest>(res),k));
}



