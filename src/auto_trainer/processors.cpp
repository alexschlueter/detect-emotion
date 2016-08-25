#include "processors.h"
#include <sstream>
#include <random>

/*============================================
=            RandomJitterExpander            =
============================================*/

RandomJitterExpander::RandomJitterExpander(double meanx, double stdx, double meany, double stdy)
    :_meanx(meanx),_meany(meany),_stdx(stdx),_stdy(stdy){
    // empty
}

void RandomJitterExpander::analyse(const CloudAction &){
    // empty
}

std::string  RandomJitterExpander::name() const{
    stringstream str;
    str <<  "RandomJitterExpander";
    str << "_meanx="<<_meanx;
    str << "_stdx="<<_stdx;
    str << "_meany="<<_meany;
    str << "_stdy="<<_stdy;
    return str.str();
}

void RandomJitterExpander::save(const string &) const{
    // empty
}

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

/*=======================================
=            CloudNormalizer            =
=======================================*/

void CloudNormalizer::analyse(const CloudAction &){
    // empty
}

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

void CloudNormalizer::save(const std::string & filename) const{
    // empty
}


/*=========================================
=            PCAFeatureReducer            =
=========================================*/

PCAFeatureReducer::PCAFeatureReducer(double retain_variance):_retain_variance(retain_variance){
    // empty
}

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

std::unique_ptr<PCAFeatureReducer> PCAFeatureReducer::load(const string &filename){
    std::unique_ptr<PCAFeatureReducer> res(new PCAFeatureReducer(0));
    res->_pca.fromFile(filename);
    return res;
}

/*=======================================
=            FeatureShuffler            =
=======================================*/

void FeatureShuffler::analyse(const FeatureTruth &){
    // empty
}
std::string FeatureShuffler::name() const{
    return "FeatureShuffler";
}
void FeatureShuffler::save(const string &filename) const{
    // empty
}
FeatureTruth FeatureShuffler::apply(const FeatureTruth & ft) const{
    return ft.shuffled();
}

/*===============================================
=            FeatureMinMaxNormalizer            =
===============================================*/

void FeatureMinMaxNormalizer::analyse(const FeatureTruth & features){
    _scaler = FeatureScaling(features._features);
}

std::string FeatureMinMaxNormalizer::name() const{
    return "FeatureNormalizer";
}

FeatureTruth FeatureMinMaxNormalizer::apply(const FeatureTruth & f) const{
    cv::Mat newFeatures = f._features.clone();
    _scaler.scale(newFeatures);
    return FeatureTruth(newFeatures,f._truth);
}

void FeatureMinMaxNormalizer::save(const string &filename) const{
    cv::FileStorage storage(filename,cv::FileStorage::WRITE);
    storage << "max"<<_scaler.colMax;
    storage << "min"<<_scaler.colMax;
    storage.release();
}

std::unique_ptr<FeatureMinMaxNormalizer> FeatureMinMaxNormalizer::load(const string &filename) {
    cv::FileStorage storage(filename,cv::FileStorage::READ);
    if (!storage.isOpened()) return nullptr;
    auto res =  std::unique_ptr<FeatureMinMaxNormalizer>(new FeatureMinMaxNormalizer());
    try{
        storage["max"] >> res->_scaler.colMax;
        storage["min"] >> res->_scaler.colMax;
    }catch (...){
        storage.release();
        return nullptr;
    }
    storage.release();
    return res;
}

/*=======================================
=            ReduceNegatives            =
=======================================*/

ReduceNegatives::ReduceNegatives(double negativesToPositives): _negativesToPostives(negativesToPositives){
    // empty
}

std::string ReduceNegatives::name() const{
    return std::string("ReduceNegatives_negToPos=")+std::to_string(_negativesToPostives);
}

void ReduceNegatives::analyse(const FeatureTruth &){
    // empty
}

FeatureTruth ReduceNegatives::apply(const FeatureTruth &f ) const{
    auto negatives = f.negativeSamples();
    auto positives = f.positiveSamples();
    if (negatives.size() < positives.size()*_negativesToPostives){
        return f;
    }
    return positives.added(negatives.subset(0,positives.size()*_negativesToPostives));
}

void ReduceNegatives::save(const string &filename) const{
    // empty
}


/*=================================
=            CloudMask            =
=================================*/

CloudMask::CloudMask(ifstream &file){
    int lm;
    while (file >> lm) {
        _toKeep.push_back(lm);
    }
}

void CloudMask::analyse(const CloudAction &){
    // empty
}

CloudAction CloudMask::apply(const CloudAction & cloud) const{
    VideoList newVideos;
    for (const Video & v: cloud._landmarks){
        Video newVideo = v;
        for (auto && cloud: newVideo){
            for (auto v: _toKeep){
                cloud[v] = cv::Point2f(0,0);
            }
        }
        newVideos.push_back(newVideo);
    }
    return CloudAction(std::move(newVideos), cloud._actionUnits);
}

void CloudMask::save(const string &filename) const{
    // empty
}

std::string CloudMask::name() const{
    std::stringstream str;
    str << "MaskFeatureExtraction";
    for (int i: _toKeep){
        str << "_" << std::to_string(i);
    }
    return str.str();
}

/*======================================
=            PersonShuffler            =
======================================*/

void PersonShuffler::analyse(const CloudAction &){
    // empty
}

std::string PersonShuffler::name() const{
    return "PersonShuffler";
}

void PersonShuffler::save(const string &filename) const{
    // empty
}


CloudAction PersonShuffler::apply(const CloudAction & cloud) const{
    std::vector<size_t> permutation;
    permutation.reserve(cloud._landmarks.size());
    for (size_t i=0; i< cloud._landmarks.size(); i++){
        permutation.push_back(i);
    }
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(permutation.begin(),permutation.end(),generator);
    VideoList newVideos;
    newVideos.reserve(cloud._landmarks.size());
    vector<ActionUnit> newActionUnits;
    newActionUnits.reserve(cloud._actionUnits.size());
    for(size_t i : permutation){
        newActionUnits.push_back(cloud._actionUnits[i]);
        newVideos.push_back(cloud._landmarks[i]);
    }
    return CloudAction(std::move(newVideos),std::move(newActionUnits));
}

/*======================================
=           PCACloudReducer            =
======================================*/

PCACloudReducer::PCACloudReducer(unsigned int dimension):_dimension(dimension){}

void PCACloudReducer::analyse(const CloudAction & c){
    Video flatten;
    for (const Video & v: c._landmarks){
        flatten.insert(flatten.end(),v.begin(),v.end());
    }
    _pca = std::unique_ptr<PCA_Result<66>>(new PCA_Result<66>(std::move(computePCA<66>(flatten))));
}

std::string PCACloudReducer::name() const{
    return std::string("PCA-Cloud-Reducer_outdim=")+std::to_string(_dimension);
}

CloudAction PCACloudReducer::apply(const CloudAction & cloud) const{
    VideoList newVideos;
    for (const Video & v: cloud._landmarks){
        Video newVideo = v;
        for (auto && points: newVideo){
            points = _pca->rebuild(points,_dimension);
        }
        newVideos.push_back(newVideo);
    }
    return CloudAction(std::move(newVideos), cloud._actionUnits);
}

void PCACloudReducer::save(const string &filename) const{
    cv::FileStorage storage(filename,cv::FileStorage::WRITE);
    storage << "outdim" << static_cast<int>(_dimension);
    _pca->save(storage);
    storage.release();
}

std::unique_ptr<PCACloudReducer> PCACloudReducer::load(const string &filename) {
    cv::FileStorage storage(filename,cv::FileStorage::READ);
    if (!storage.isOpened()) return nullptr;
    try{
        int dim;
        storage["outdim"] >> dim;
        if (dim < 1 || dim > 66) return nullptr;
        auto res =  std::unique_ptr<PCACloudReducer>(new PCACloudReducer(dim));
        res->_pca = std::unique_ptr<PCA_Result<66>>(new PCA_Result<66>(PCA_Result<66>::load(storage)));
        storage.release();
        return res;
    }catch(...){
        storage.release();
        return nullptr;
    }
}
