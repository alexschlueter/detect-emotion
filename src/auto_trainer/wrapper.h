#ifndef WRAPPER_H
#define WRAPPER_H
#include "classifier.h"
#include "util.h"
#include "featureextraction.h"


class FeatureTruth{
public:
    FeatureTruth();
    FeatureTruth(cv::Mat features, cv::Mat truth);
    FeatureTruth shuffled() const;
    FeatureTruth added(const FeatureTruth & other) const;
    FeatureTruth positiveSamples() const;
    FeatureTruth negativeSamples() const;
    int size() const;
    FeatureTruth subset(int beg, int end) const;

    inline bool isEmpty() const{
        return _features.cols == 0 || _features.rows == 0;
    }

    cv::Mat _features;
    cv::Mat _truth;
private:
    FeatureTruth sampleTruth(ClassifierResult) const;
};

class CloudAction{
public:
    inline CloudAction(VideoList && land, vector<ActionUnit> && action): _landmarks(land),_actionUnits(action){}
    inline CloudAction(const VideoList & land, const vector<ActionUnit> & action): _landmarks(land),_actionUnits(action){}
    inline CloudAction(VideoList && land, const vector<ActionUnit> & action): _landmarks(land),_actionUnits(action){}
    inline size_t size() const{
        return _landmarks.size();
    }

    inline CloudAction subset(int beg, int end) const{
        vector<Video> l2;
        vector<ActionUnit> a2;
        l2.insert(l2.end(),_landmarks.begin()+beg, _landmarks.begin()+end);
        a2.insert(a2.end(),_actionUnits.begin()+beg, _actionUnits.begin()+end);
        return CloudAction{l2, a2};
    }


    FeatureTruth extractFrameFeature(const FeatureExtractionBase<66>& extractor, const string & actionname, int threshold) const;
    FeatureTruth extractTimeFeature(const TimeFeatureExtractionBase<66>& extractor, const string & actionname, int threshold) const;

    inline CloudAction appened(const CloudAction & other) const{
       VideoList lands;
       lands.reserve(_landmarks.size() + other._landmarks.size());
       lands.insert(lands.end(),_landmarks.begin(),_landmarks.end());
       lands.insert(lands.end(),other._landmarks.begin(),other._landmarks.end());

       vector<ActionUnit> actions;
       actions.reserve(_actionUnits.size() + other._actionUnits.size());
       actions.insert(actions.end(),_actionUnits.begin(),_actionUnits.end());
       actions.insert(actions.end(),other._actionUnits.begin(),other._actionUnits.end());
       return CloudAction(std::move(lands),std::move(actions));

    }

    inline CloudAction clone() const{
        return CloudAction{_landmarks,_actionUnits};
    }

    VideoList _landmarks;
    vector<ActionUnit> _actionUnits;
};

#endif // WRAPPER_H
