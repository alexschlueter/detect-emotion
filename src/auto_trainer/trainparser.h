#ifndef TRAINPARSER_H
#define TRAINPARSER_H
#include "wrapper.h"
#include "pointcloud.h"
#include "classifier.h"
#include <opencv2/core.hpp>
#include "featureextraction.h"
#include "processors.h"
#include "util.h"

template <typename T>
using _Ptr = std::unique_ptr<T>;

using CloudProcessorP = _Ptr<CloudProcessor>;
using FeatureProcessorP = _Ptr<FeatureProcessor>;
using ClassifierConstrP = _Ptr<ClassifierConstructor>;
using FrameFeatureExtractionP = _Ptr<FeatureExtractionBase<66>>;
using TimeFeatureExtractionP = _Ptr<TimeFeatureExtractionBase<66>>;
struct TimeFeatureProcessor{
    TimeFeatureExtractionP extractor;
    vector<FeatureProcessorP> processors;
};
struct FrameFeatureProcessor{
    FrameFeatureExtractionP extractor;
    vector<FeatureProcessorP> processors;
};

using namespace std;
struct ParseResult{
    string landmarkdir;
    string actiondir;
    vector<string> actionnames;
    float training_percent;
    vector<CloudProcessorP> cloud_processor;
    vector<ClassifierConstrP> classifier;
    vector<FrameFeatureProcessor> frame_features_processors;
    vector<TimeFeatureProcessor> time_features_processors;
    int action_threshold;
};

ParseResult parseConfig(const std::string & filename);

#endif // TRAINPARSER_H
