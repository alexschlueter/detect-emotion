#include "classifier.h"
#include <sstream>
#include <iostream>

class RandomForestClassifier: public Classifier{
public:
    explicit RandomForestClassifier(std::unique_ptr<CvRTrees> && rtree, CvRTParams param): _rtree(std::move(rtree)),_param(param){}

    virtual ClassifierResult classify(const cv::Mat & feature) const{
        int label = static_cast<int>(_rtree->predict(feature));
        if (label == 1) return action;
        if (label == 0) return noaction;
        return failure;
    }
    virtual bool serialize(const std::string & filename) const {
        _rtree->save(filename.c_str());
        return true;
    }

    virtual  std::string name() const{
     auto params = _param;
     std::stringstream str;
     str << "random_forest";
     str << "_maxCategories="<<params.max_categories;
     str << "_maxDepth="<<params.max_depth;
     str << "_min_sample_count="<<params.min_sample_count;
     str << "_regression_accuracy="<<params.regression_accuracy;
     str << "_truncate_pruned_tree="<<params.truncate_pruned_tree;
     return str.str();
    }
private:
    std::unique_ptr<CvRTrees> _rtree;
    CvRTParams _param;
};
RandomForestConstructor::RandomForestConstructor(CvRTParams params):_params(params){}
RandomForestConstructor::RandomForestConstructor(){}

std::unique_ptr<Classifier> RandomForestConstructor::train(const cv::Mat & trainingsset, const cv::Mat& truthset) const{
    auto res = std::unique_ptr<CvRTrees>(new CvRTrees());
    assert(trainingsset.rows == truthset.rows);
    assert(truthset.cols == 1);

    cv::Mat truthAsFloat;
    truthset.convertTo(truthAsFloat, CV_32F);
    for(int i=0; i< truthset.rows; i++){
      if (truthAsFloat.at<float>(i,0)==noaction){
          truthAsFloat.at<float>(i,0)= 0;
      }else if(truthAsFloat.at<float>(i,0) == action){
          truthAsFloat.at<float>(i,0) = 1;
      }else{
          std::cerr << "[WARNING] Invalid truth value given to classifier"<<std::endl;
      }
    }
    res->train(trainingsset, CV_ROW_SAMPLE, truthAsFloat, cv::Mat(),cv::Mat(),cv::Mat(),cv::Mat(),_params);
    return std::unique_ptr<Classifier>(new RandomForestClassifier(std::move(res),_params));

}

std::unique_ptr<Classifier> RandomForestConstructor::deserialize(const std::string & filename) const{
    auto res = std::unique_ptr<CvRTrees>(new CvRTrees());
    res->load(filename.c_str());
    return std::unique_ptr<Classifier>(new RandomForestClassifier(std::move(res),CvRTParams()));
}
