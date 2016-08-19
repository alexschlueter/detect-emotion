#include "wrapper.h"
#include <random>
#include <opencv2/core.hpp>

#include <iostream>
FeatureTruth::FeatureTruth(){ }

FeatureTruth::FeatureTruth(cv::Mat actions, cv::Mat truth):_features(actions),_truth(truth){}

FeatureTruth FeatureTruth::added(const FeatureTruth &other) const{
    if (_features.cols != other._features.cols) return FeatureTruth();
    if (_features.cols == 0) return FeatureTruth();
    if (_features.rows == 0) return FeatureTruth();
    cv::Mat newActions;
    cv::Mat newTruth;
    cv::vconcat(_features,other._features,newActions);
    cv::vconcat(_truth,other._truth,newTruth);
    assert(newActions.cols == _features.cols);
    assert(newActions.rows == _features.rows+other._features.rows);
    assert(newTruth.cols == 1);
    assert(newTruth.rows == newActions.rows);
    return FeatureTruth(newActions, newTruth);
}

FeatureTruth FeatureTruth::sampleTruth(ClassifierResult res) const{
    std::vector<size_t> idxs;
    for (size_t i=0; i< _truth.rows; i++){
        if (_truth.at<float>(i,0) == static_cast<float>(res)){
            idxs.push_back(i);
        }
    }
    cv::Mat newActions(idxs.size(), _features.cols, _features.type());
    cv::Mat newTruth(idxs.size(), 1, _truth.type());
    int i=0;
    for(auto idx: idxs){
        _features.row(idx).copyTo(newActions.row(i));
        _truth.row(idx).copyTo(newTruth.row(i));
        i++;
    }
    return FeatureTruth(newActions, newTruth);
}

FeatureTruth FeatureTruth::positiveSamples() const{
   return sampleTruth(action) ;
}
FeatureTruth FeatureTruth::negativeSamples() const{
   return sampleTruth(noaction) ;
}

FeatureTruth FeatureTruth::shuffled() const{
    std::vector<size_t> permutation;
    permutation.reserve(_features.rows);
    for (size_t i=0; i< _features.rows; i++){
        permutation.push_back(i);
    }
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(permutation.begin(),permutation.end(),generator);
    cv::Mat newActions(_features.rows, _features.cols, _features.type());
    cv::Mat newTruth(_truth.rows, 1, _truth.type());
    for(size_t i=0; i<permutation.size();i++){
        _truth.row(permutation[i]).copyTo(newTruth.row(i));
        _features.row(permutation[i]).copyTo(newActions.row(i));
    }
    return FeatureTruth(newActions,newTruth );
}

int FeatureTruth::size() const{
   return _truth.rows;
}


FeatureTruth FeatureTruth::subset(int beg, int end) const{
    if (beg > end || end > size()) return FeatureTruth();
    cv::Mat newActions(end-beg,_features.cols, _features.type());
    cv::Mat newTruth(end-beg,1, _truth.type());
    for(size_t i = 0; i < end-beg; i++){
        _features.row(beg+i).copyTo(newActions.row(i));
        _truth.row(beg+i).copyTo(newTruth.row(i));
    }
    return FeatureTruth(newActions, newTruth);
}

inline cv::Mat extractAction(const std::vector<ActionUnit>& units, const string & actionname, int threshold){
    std::vector<bool> actions;
    for(const ActionUnit & unit: units){
        cv::Mat actions_mat;
        if (!unit.getActionByName(actionname, actions_mat)) return cv::Mat();
        for(size_t i=0; i< actions_mat.rows; i++){
            actions.push_back(actions_mat.at<uint8_t>(i,0) > threshold);
        }
    }
    cv::Mat res(actions.size(),1, CV_32FC1);
    for(size_t i=0; i< actions.size(); i++){
        res.at<float>(i,0) = actions[i] ? action : noaction;
    }
    return res;
}

FeatureTruth CloudAction::extractFrameFeature(const FeatureExtractionBase<66> &extractor, const string &actionname, int threshold) const{
    cv::Mat resFeatures;
    for(auto && video: this->_landmarks){
        if (resFeatures.empty()){
            resFeatures = extractFeaturesFromData<66>(video,extractor);
        }else{
            cv::vconcat(resFeatures,extractFeaturesFromData<66>(video,extractor),resFeatures);
        }
    }
    cv::Mat resAction = extractAction(_actionUnits,actionname,threshold);
    return FeatureTruth(resFeatures,resAction);
}

FeatureTruth CloudAction::extractTimeFeature(const TimeFeatureExtractionBase<66> &extractor, const string &actionname, int threshold) const{
    cv::Mat resFeatures;
    for(auto && video: this->_landmarks){
        if (resFeatures.empty()){
            resFeatures = extractTimeFeaturesFromData<66>(video,extractor);
        }else{
            cv::vconcat(resFeatures,extractTimeFeaturesFromData<66>(video,extractor),resFeatures);
        }
    }

    cv::Mat resActions;
    for(auto && unit: _actionUnits){
        std::vector<ActionUnit> unitV{unit};
        cv::Mat curVideoTruth = extractAction(unitV, actionname, threshold);
        cv::Mat timeTruth = extractTimeTruth<66>(curVideoTruth,extractor);
        if (resActions.empty()){
            resActions = timeTruth;
        }else{
            cv::vconcat(resActions,timeTruth,resActions);
        }
    }
    return FeatureTruth(resFeatures,resActions);
}
