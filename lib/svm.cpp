#include "classifier.h"
#include <opencv2/ml/ml.hpp>

#include <iostream>
#include <sstream>

class SVMClassifier: public Classifier{
public:
    explicit SVMClassifier(std::unique_ptr<CvSVM> && svm): _svm(std::move(svm)){}

    virtual ClassifierResult classify(const cv::Mat & feature) const{
        return _svm->predict(feature) < 0 ? noaction : action;
    }
    virtual bool serialize(const std::string & filename) const {
        _svm->save(filename.c_str());
        return true;
    }

    virtual std::string name() const{
        CvSVMParams param = _svm->get_params();
        std::stringstream str;
        str << "svm_";
        if (param.kernel_type == CvSVM::LINEAR){
           str << "linear" ;
        }else if (param.kernel_type == CvSVM::POLY){
            str << "polynom";
            str << "_coef0="<<param.coef0;
            str << "_gamma="<<param.gamma;
            str << "_degree"<<param.degree;
        }else if (param.kernel_type == CvSVM::RBF){
            str << "rbf";
            str << "_gamma="<<param.gamma;
        }else if (param.kernel_type == CvSVM::SIGMOID){
            str << "sigmoid";
            str << "_coef0="<<param.coef0;
            str << "_gamma="<<param.gamma;
        }
        return str.str();
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
    assert(trainingsset.type() == CV_32FC1);
    assert(truthset.cols == 1);
    //Transform Matrix for SVM
    cv::Mat truthAsFloat;
    truthset.convertTo(truthAsFloat, CV_32F);
    for(int i=0; i< truthset.rows; i++){
      if (truthAsFloat.at<float>(i,0) == noaction){
          truthAsFloat.at<float>(i,0)=-1;
      }else if(truthAsFloat.at<float>(i,0) == action){
          truthAsFloat.at<float>(i,0) = 1;
      }else{
          std::cerr << "[WARNING] Invalid truth value given to classifier"<<std::endl;
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



