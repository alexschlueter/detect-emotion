#include "trainparser.h"
#include "classifier.h"
#include <QtCore/QJsonObject>
#include <QJsonArray>
#include <functional>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
using namespace std;

template <typename F>
auto parseArray(const QJsonArray & arr, F func) ->  std::vector<decltype(func(arr))> {
    std::vector<decltype(func(arr))> res;
    res.reserve(arr.size());
    for (auto val: arr){
        res.emplace_back(func(val));
    }
    return res;
}

#define arrayf(f) [](const QJsonValue & a){return f(a.toObject());}
#define arrayf2(f,reldir) [reldir](const QJsonValue & a){return f(a.toObject(),reldir);}

#include  <initializer_list>
inline void lookupkeys(const QJsonObject & obj, std::initializer_list<string> keys){
    for (auto && key: keys){
       if (!obj.contains(QString::fromStdString(key))){
           cerr << "[ERROR] Key not set in config: "<<key<<endl;
           cerr << "[Info] Available keys: ";
           for (auto && key: keys){
               cerr << key << ",";
           }
           cerr << endl;
           exit(EXIT_FAILURE);
       }
    }
}
inline void lookupfile(const string & filename){
    if (!exists(filename)){
        cerr << "[ERROR] "<<filename << " does not exists"<<endl;
        exit(EXIT_FAILURE);
    }
}


FrameFeatureExtractionP parseFrameFeature(const QJsonObject & );
TimeFeatureExtractionP parseTimeFeature(const QJsonObject & );
ClassifierConstrP parseClassifierConstructor(const QJsonObject & );
ClassifierP parseClassifier(const QJsonObject &obj, const string &reldir);
FeatureProcessorP parseFeatureProcessor(const QJsonObject &obj, const string &reldir);
CloudProcessorP parseCloudProcessor(const QJsonObject &obj, const string &reldir );

template <typename T>
using ParseMap = std::map<string,std::function<T(const QJsonObject&)>>;
template <typename T>
using LoaderMap = std::map<string,std::function<T(const string &,const QJsonObject&)>>;

template <typename K, typename T>
inline void lookupname(const std::map<K,T> &map, const std::string & name){
    if (map.find(name) == map.end()){
           cerr << "[ERROR] Unknow name: "<<name<<endl;
           cerr << "[INFO] Available names: ";
           for(auto it = map.begin(); it != map.end(); it++){
               cerr << it->first<<",";
           }
           cerr << endl;
           exit(EXIT_FAILURE);
    }
}

static LoaderMap<FeatureProcessor*> featureProcessor_loader = {
   {"pcareducer", [](const string & filename, const QJsonObject &){
        lookupfile(filename);
        auto res =  PCAFeatureReducer::load(filename).release();
        if (res == nullptr){
            cerr << "[ERROR] Unable to load pca-feature-processor "<<filename<<endl;
            exit(EXIT_FAILURE);
        }
        return res;
    }},
   {"minmaxnormalize", [](const string & filename,const QJsonObject&){
        lookupfile(filename);
        auto res = FeatureMinMaxNormalizer::load(filename).release();
        if (res == nullptr){
            cerr << "[ERROR] Unable to load minmaxnormalize-feature-processor "<<filename<<endl;
            exit(EXIT_FAILURE);
        }
        return res;
    }},
};

static ParseMap<FeatureProcessor*> featureProcessor_parser = {
        {"pcareducer", [](const QJsonObject & obj){
                 lookupkeys(obj,{"retain_variance"});
                 double var = obj["retain_variance"].toDouble();
                 return new PCAFeatureReducer(var);
         }},
        {"shuffle", [](const QJsonObject&){ return new FeatureShuffler();}},
        {"minmaxnormalize", [](const QJsonObject&){ return new FeatureMinMaxNormalizer();}},
        {"reducenegatives", [](const QJsonObject& obj){
                 lookupkeys(obj,{"negativeToPositives"});
                 double negToPos = obj["negativeToPositives"].toDouble();
                 return new ReduceNegatives(negToPos);
         }}
};

static LoaderMap<CloudProcessor*> cloudProcessor_loader = {
   {"pcareducer", [](const string & filename, const QJsonObject &){
        lookupfile(filename);
        auto res = PCACloudReducer::load(filename).release();
        if (res == nullptr){
            cerr << "[ERROR] Unable to load pca-cloud-processor "<<filename<<endl;
            exit(EXIT_FAILURE);
        }
        return res;
    }}
};

static ParseMap<CloudProcessor*> cloudProcessor_parser = {
        {"randomjitterexpander", [](const QJsonObject& obj){
                 lookupkeys(obj,{"meanx","meany","stdx","stdy"});
                 double meanx = obj["meanx"].toDouble();
                 double meany = obj["meany"].toDouble();
                 double stdx = obj["stdx"].toDouble();
                 double stdy = obj["stdy"].toDouble();
                 return new RandomJitterExpander(meanx,stdx,meany,stdy);
         }},
        {"pointcloudnormalization", [](const QJsonObject){return new CloudNormalizer;}},
        {"mask", [](const QJsonObject & obj){
                 if (obj.contains("filename")){
                 lookupkeys(obj,{"filename"});
                 auto filename = obj["filename"].toString().toStdString();
                 lookupfile(filename);
                 std::ifstream stream(filename);
                 return new CloudMask(stream);
               }else if (obj.contains("tokeep")){
                QJsonArray tokeep = obj["tokeep"].toArray();
                 vector<int> res;
                 res.reserve(tokeep.size());
                 for (auto val: tokeep){res.push_back(val.toInt());}
                 return new CloudMask(res);
             }else{
                 cerr << "[ERROR] Mask-Cloud-Processor need filename of text-file or 'tokeep' argument"<<endl;
                 exit(EXIT_FAILURE);
             }
         }},
       {"personshuffler", [](const QJsonObject & obj){
            return new PersonShuffler();
        }},
        {"pcareducer", [](const QJsonObject & obj){
             lookupkeys(obj,{"reducing_dimension"});
             auto dim = obj["reducing_dimension"].toInt();
             if (dim < 1 || dim > 66){
                 cerr << "[ERROR] Invalid reducing dimension: "<<dim<<endl;
                 cerr << "[INFO] Valid are values between 1-66"<<endl;
             }
             return new PCACloudReducer(dim);
         }}
};


static ParseMap<TimeFeatureExtractionBase<66>*> timeFeature_parser = {
        {"differential",
         [](const QJsonObject & obj){
                 lookupkeys(obj,{"base", "truth_threshold"});
                 auto baseFeature = std::shared_ptr<FeatureExtractionBase<66>>(parseFrameFeature(obj["base"].toObject()));
                 auto truth_threshold = obj["truth_threshold"].toDouble();
                 return new SimpleTimeDifferentialExtraction<66>(baseFeature, truth_threshold);
         } }
};

static ParseMap<FeatureExtractionBase<66>*> frameFeature_parser = {
        {"xy", [](const QJsonObject & obj){return new XYFeatureExtraction<66>(); }},
        {"interpolation", [](const QJsonObject & obj){return new InterpolationFeatureExtraction();}}
};

ClassifierConstructor* parseSVM (const QJsonObject & data);

#define classifier_loader_entry(name, classname) {#name, [](const string & filename,const QJsonObject &){return classname().deserialize(filename);}}
static LoaderMap<unique_ptr<Classifier>> classifier_loader = {
classifier_loader_entry(svm,SVMConstructor),
classifier_loader_entry(randomforest,RandomForestConstructor)
};

static ParseMap<ClassifierConstructor*> classifier_parser = {
        {"svm", &parseSVM},
        {"randomforest", [](const QJsonObject & obj){
             lookupkeys(obj,{"max_depth","rand_subset_size","number_of_trees"});
             int max_depth = obj["max_depth"].toInt();
             int rand_subset_size = obj["rand_subset_size"].toInt();
             int number_of_trees = obj["number_of_trees"].toInt();
             CvRTParams params(max_depth,
                      2, // min_sample_count
                      0.0f, // regression_accuracy
                      false, // use_surrogates
                      2, //  max_categories - not used for 2 class problem
                      nullptr, // priors
                      false, //calc_var_importance (not needed)
                      rand_subset_size, // nactive_vars
                      number_of_trees, //max_num_of_trees_in_the_fores
                      0, //forest_accuracy, 0 since not used, because terminate if all trees created
                      CV_TERMCRIT_ITER //termcrit_type
                      );
             return new RandomForestConstructor(params);
         }}
};


TimeFeatureExtractionP parseTimeFeature(const QJsonObject & obj ){
    lookupkeys(obj,{"name"});
    auto name = obj["name"].toString().toLower().toStdString();
    lookupname(timeFeature_parser,name);
    return  TimeFeatureExtractionP(timeFeature_parser[name](obj));
}

TimeFeatureProcessor parseTimeFeatureProcessor(const QJsonObject & obj, const string & reldir){
    lookupkeys(obj,{"processors"});
    return TimeFeatureProcessor{
        parseTimeFeature(obj),
        parseArray(obj["processors"].toArray(),arrayf2(parseFeatureProcessor, reldir))
    };
}


FrameFeatureExtractionP parseFrameFeature(const QJsonObject & obj ){
    lookupkeys(obj,{"name"});
    auto name = obj["name"].toString().toLower().toStdString();
    lookupname(frameFeature_parser,name);
    return  FrameFeatureExtractionP(frameFeature_parser[name](obj));
}

FrameFeatureProcessor parseFrameFeatureProcessor(const QJsonObject & obj, const string & reldir){
    lookupkeys(obj,{"processors"});
    return FrameFeatureProcessor{
        parseFrameFeature(obj),
        parseArray(obj["processors"].toArray(),arrayf2(parseFeatureProcessor,reldir))
    };
}

FeatureProcessorP parseFeatureProcessor(const QJsonObject &obj, const string & reldir ){
    lookupkeys(obj,{"name"});
    auto name = obj["name"].toString().toLower().toStdString();
    if (obj.contains("load")){
       lookupname(featureProcessor_parser,name);
       auto filename = absoluteTo(reldir,obj["load"].toString().toStdString());
       lookupfile(filename);
       return FeatureProcessorP(featureProcessor_loader[name](filename,obj));
    }else{
        lookupname(featureProcessor_parser,name);
        return FeatureProcessorP(featureProcessor_parser[name](obj));
    }
}
CloudProcessorP parseCloudProcessor(const QJsonObject &obj, const string & reldir ){
    lookupkeys(obj,{"name"});
    auto name = obj["name"].toString().toLower().toStdString();
    if (obj.contains("load")){
       lookupname(cloudProcessor_loader,name);
       auto filename = absoluteTo(reldir,obj["load"].toString().toStdString());
       lookupfile(filename);
       return CloudProcessorP(cloudProcessor_loader[name](filename,obj));
    }else{
        lookupname(cloudProcessor_parser,name);
        return CloudProcessorP(cloudProcessor_parser[name](obj));
    }
}

ClassifierConstructor* parseSVM (const QJsonObject & data){
    CvSVMParams params;
    params.term_crit.max_iter = data.contains("iterations") ? data["iterations"].toInt() : 1000;
    params.svm_type = CvSVM::C_SVC;
    lookupkeys(data,{"type"});
    auto type = data["type"].toString().toLower();
    if (type == "linear" ){
        params.kernel_type = CvSVM::LINEAR;
    }else if (type == "rbf"){
        lookupkeys(data,{"gamma"});
        params.kernel_type = CvSVM::RBF;
        params.gamma = data["gamma"].toDouble();
    }else if (type == "sigmoid"){
        lookupkeys(data,{"gamma","coef0"});
        params.kernel_type = CvSVM::SIGMOID;
        params.gamma = data["gamma"].toDouble();
        params.coef0 = data["coef0"].toDouble();
    }else if (type == "polynom"){
        lookupkeys(data,{"gamma","coef0","degree"});
        params.kernel_type = CvSVM::POLY;
        params.gamma = data["gamma"].toDouble();
        params.coef0 = data["coef0"].toDouble();
        params.degree = data["degree"].toDouble();
    }
    return new SVMConstructor(params);
}


ClassifierConstrP parseClassifierConstructor(const QJsonObject & obj){
    lookupkeys(obj,{"name"});
    auto name = obj["name"].toString().toLower().toStdString();
    lookupname(classifier_parser,name);
    return ClassifierConstrP(classifier_parser[name](obj));
}

ClassifierP parseClassifier(const QJsonObject & obj, const string & reldir){
    lookupkeys(obj,{"name","load"});
    auto name = obj["name"].toString().toLower().toStdString();
    lookupname(classifier_loader,name);
    auto filename = absoluteTo(reldir,obj["load"].toString().toLower().toStdString());
    lookupfile(filename);
    return ClassifierP(classifier_loader[name](filename,obj));
}


#include <QJsonDocument>
#include <QFile>


TrainParseResult parseTrainConfig(const string &filename){
    auto reldir =dirOfFile(filename);
    auto f = QString::fromStdString(filename);
    QFile file(f);
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    file.close();
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(data, & err).object();
    if (err.error != QJsonParseError::NoError){
        cerr << "[FATAL] Error while parsing json: "<<err.errorString().toStdString() << endl;
        exit(EXIT_FAILURE);
    }
    lookupkeys(json,{"landmark_dir","action_dir","action_names",
                    "action_threshold","training_percent","classifier",
                    "cloud_processor","frame_features",
                    "time_features", "time_frame_step"
               });
    TrainParseResult res;
    res.landmarkdir = json["landmark_dir"].toString().toStdString();
    res.actiondir = json["action_dir"].toString().toStdString();
    res.actionnames = parseArray(json["action_names"].toArray(),[](const QJsonValue & a){return a.toString().toStdString();});
    res.action_threshold = json["action_threshold"].toInt();
    res.training_percent = json["training_percent"].toDouble();
    res.classifier =  parseArray(json["classifier"].toArray(),arrayf(parseClassifierConstructor));
    res.cloud_processor =  parseArray(json["cloud_processor"].toArray(),arrayf2(parseCloudProcessor,reldir));
    res.frame_features_processors = parseArray(json["frame_features"].toArray(), arrayf2(parseFrameFeatureProcessor,reldir));
    res.time_features_processors = parseArray(json["time_features"].toArray(), arrayf2(parseTimeFeatureProcessor,reldir));
    res.time_frame_step = json["time_frame_step"].toInt();
    if (res.time_frame_step < 1 ){
        cerr << "[ERROR] time_frame_step is lower than 1"<<endl;
        exit(EXIT_FAILURE);
    }
    return res;
}

EvalParseResult parseEvalConfig(const string &filename){
    auto reldir =dirOfFile(filename);
    auto f = QString::fromStdString(filename);
    QFile file(f);
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    file.close();
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(data, & err).object();
    if (err.error != QJsonParseError::NoError){
        cerr << "[FATAL] Error while parsing json: "<<err.errorString().toStdString() << endl;
        exit(EXIT_FAILURE);
    }
    lookupkeys(json,{"landmark_dir","action_dir","action_names","action_threshold","classifier", "cloud_processor"});
    if ((json.contains("time_feature") || json.contains("time_frame_step"))&& json.contains("frame_feature")){
       cerr << "[ERROR] json contains time-based config and frame-based config. Please decide for one." << endl;
       exit(EXIT_FAILURE);
    }
    EvalParseResult res;
    if (json.contains("time_feature")){
        res.kind = EvalParseResult::Kind::time;
    }else if (json.contains("frame_feature")){
        res.kind = EvalParseResult::Kind::frame;
    }else{
        cerr << "[ERROR] time_feature or frame_feature config is missing" << endl;
        exit(EXIT_FAILURE);
    }
    res.landmarkdir = json["landmark_dir"].toString().toStdString();
    res.actiondir = json["action_dir"].toString().toStdString();
    res.actionnames = parseArray(json["action_names"].toArray(),[](const QJsonValue & a){return a.toString().toStdString();});
    res.action_threshold = json["action_threshold"].toInt();
    res.classifier =  parseClassifier(json["classifier"].toObject(),reldir);
    res.cloud_processor =  parseArray(json["cloud_processor"].toArray(),arrayf2(parseCloudProcessor,reldir));
    switch (res.kind){
    case EvalParseResult::Kind::time:
        lookupkeys(json,{"time_frame_step"});
        res.frameFeature = parseFrameFeatureProcessor(json["time_feature"].toObject(),reldir);
        res.time_frame_step = json["time_frame_step"].toInt();
        if (res.time_frame_step < 1 ){
            cerr << "[ERROR] time_frame_step is lower than 1"<<endl;
            exit(EXIT_FAILURE);
        }
        break;
    case EvalParseResult::Kind::frame:
        res.frameFeature = parseFrameFeatureProcessor(json["frame_feature"].toObject(),reldir);
        break;
    }
    return res;
}

