#include "processors.h"
#include <sstream>
#include <random>

RandomJitterExpander::RandomJitterExpander(double meanx, double stdx, double meany, double stdy)
    :_meanx(meanx),_meany(meany),_stdx(stdx),_stdy(stdy){}

void RandomJitterExpander::analyse(const CloudAction &){}

std::string  RandomJitterExpander::name() const{
    stringstream str;
    str <<  "RandomJitterExpander";
    str << "_meanx="<<_meanx;
    str << "_stdx="<<_stdx;
    str << "_meany="<<_meany;
    str << "_stdy="<<_stdy;
    return str.str();
}

void RandomJitterExpander::save(const string &) const{}

Video randomGaussJittered(const Video & set, double meanx, double stdx, double meany, double stdy){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dx(meanx,stdx);
    std::normal_distribution<> dy(meany,stdy);
    Video res;
    res.reserve(set.size());
    for(auto && cloud: set){
       PointCloud<66> newCloud(cloud);
       PointArray<66> & points = newCloud.points();
       for (cv::Point2f & p: points){
           p.x += dx(gen);
           p.y += dy(gen);
       }
       res.emplace_back(std::move(newCloud));
    }
    return res;
}

CloudAction RandomJitterExpander::apply(const CloudAction & cloud) const{
    VideoList videos;
    std::vector<ActionUnit> actions;
    for (int i=0; i< cloud.size(); i++){
        videos.push_back(randomGaussJittered(cloud._landmarks[i],_meanx,_stdx,_meany,_stdy));
        actions.push_back(cloud._actionUnits[i]);
    }
    return cloud.appened(CloudAction(std::move(videos),std::move(actions)));
}

void CloudNormalizer::analyse(const CloudAction &){}

CloudAction CloudNormalizer::apply(const CloudAction & cloud) const{
    VideoList videos = cloud._landmarks;
    for (auto && video: videos){
        applyOnNormalizationOnPointCloud(video,&centerPointCloud<66>);
        applyOnNormalizationOnPointCloud(video,&rotatePointCloudOnEyeAndForehead<66>);
        applyOnNormalizationOnPointCloud(video,[](const PointCloud<66> & points){
            return scalePointCloud2(points,1.0);
        });
    }
    return CloudAction(std::move(videos),cloud._actionUnits);
}

std::string CloudNormalizer::name() const{
    return "PointCloudNormalization";
}

void CloudNormalizer::save(const std::string & filename) const{}

PCAFeatureReducer::PCAFeatureReducer(double retain_variance):_retain_variance(retain_variance){}

std::string PCAFeatureReducer::name() const {
    return "PCAFeatureReducer_retainVariance="+std::to_string(_retain_variance);
}

void PCAFeatureReducer::analyse(const FeatureTruth & feature){
    _pca = PCAnalysis(feature._features,_retain_variance);
}

void PCAFeatureReducer::save(const string &filename) const{
    _pca.toFile(filename);
}

FeatureTruth PCAFeatureReducer::apply(const FeatureTruth & feature) const{
    auto new_features = _pca.project(feature._features);
    return FeatureTruth(new_features,feature._truth);
}

void FeatureShuffler::analyse(const FeatureTruth &){}
std::string FeatureShuffler::name() const{
    return "FeatureShuffler";
}
void FeatureShuffler::save(const string &filename) const{}
FeatureTruth FeatureShuffler::apply(const FeatureTruth & ft) const{
    return ft.shuffled();
}
