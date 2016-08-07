#include "classifier.h"
class RandomForestClassifier: public Classifier{
public:
    explicit RandomForestClassifier(std::unique_ptr<CvRTrees> && rtree): _rtree(std::move(rtree)){}

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
private:
    std::unique_ptr<CvRTrees> _rtree;
};
RandomForestConstructor::RandomForestConstructor(CvRTParams params):_params(params){}

std::unique_ptr<Classifier> RandomForestConstructor::train(const cv::Mat & trainingsset, const cv::Mat& truthset) const{
    auto res = std::unique_ptr<CvRTrees>(new CvRTrees());
    assert(trainingsset.rows == truthset.rows);
    assert(truthset.cols == 1);

    cv::Mat truthAsFloat;
    truthset.convertTo(truthAsFloat, CV_32F);
    for(int i=0; i< truthset.rows; i++){
      if (truthAsFloat.at<float>(i,0)==noaction){
          truthAsFloat.at<float>(i,0)= 0;
      }else{
          truthAsFloat.at<float>(i,0) = 1;
      }
    }
    res->train(trainingsset, CV_ROW_SAMPLE, truthAsFloat, cv::Mat(),cv::Mat(),cv::Mat(),cv::Mat(),_params);
    return std::unique_ptr<Classifier>(new RandomForestClassifier(std::move(res)));

}

std::unique_ptr<Classifier> RandomForestConstructor::deserialize(const std::string & filename) const{
    auto res = std::unique_ptr<CvRTrees>(new CvRTrees());
    res->load(filename.c_str());
    return std::unique_ptr<Classifier>(new RandomForestClassifier(std::move(res)));
}
