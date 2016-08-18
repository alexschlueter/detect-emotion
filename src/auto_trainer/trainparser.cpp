#include "trainparser.h"
#include "classifier.h"
#include <QtCore/QJsonObject>
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
using namespace std;

FrameFeatureExtractionP parseFrameFeature(const QJsonObject & );
TimeFeatureExtractionP parseTimeFeature(const QJsonObject & );
ClassifierConstrP parseClassifier(const QJsonObject & );
FeatureProcessorP parseFeatureProcessor(const QJsonObject &);
CloudProcessorP parseCloudProcessor(const QJsonObject & );

template <typename T>
using ParseMap = std::map<string,std::function<T*(const QJsonObject&)>>;

static ParseMap<FeatureProcessor> featureProcessor_parser = {
    {"pcareducer", [](const QJsonObject & obj){
        double var = obj["retain_variance"].toDouble();
        return new PCAFeatureReducer(var);
     }},
    {"shuffle", [](const QJsonObject&){ return new FeatureShuffler();}}
};

static ParseMap<CloudProcessor> cloudProcessor_parser = {
    {"randomjitterexpander", [](const QJsonObject& obj){
        double meanx = obj["meanx"].toDouble();
        double meany = obj["meany"].toDouble();
        double stdx = obj["stdx"].toDouble();
        double stdy = obj["stdy"].toDouble();
        return new RandomJitterExpander(meanx,stdx,meany,stdy);
    }},
    {"pointcloudnormalization", [](const QJsonObject){return new CloudNormalizer;}}
};


static ParseMap<TimeFeatureExtractionBase<66>> timeFeature_parser = {
        {"differential",
         [](const QJsonObject & obj){
            auto baseFeature = std::shared_ptr<FeatureExtractionBase<66>>(parseFrameFeature(obj["base"].toObject()));
            return new SimpleTimeDifferentialExtraction<66>(baseFeature);
          } }
};

static ParseMap<FeatureExtractionBase<66>> frameFeature_parser = {
        {"xy", [](const QJsonObject & obj){return new XYFeatureExtraction<66>(); }}
};

ClassifierConstructor* parseSVM (const QJsonObject & data);

static ParseMap<ClassifierConstructor> classifier_parser = {
        {"svm", &parseSVM}
};


TimeFeatureExtractionP parseTimeFeature(const QJsonObject & obj ){
    return  TimeFeatureExtractionP(timeFeature_parser[obj["name"].toString().toLower().toStdString()](obj));
}

FrameFeatureExtractionP parseFrameFeature(const QJsonObject & obj ){
    return  FrameFeatureExtractionP(frameFeature_parser[obj["name"].toString().toLower().toStdString()](obj));
}

FeatureProcessorP parseFeatureProcessor(const QJsonObject &obj ){
    return FeatureProcessorP(featureProcessor_parser[obj["name"].toString().toLower().toStdString()](obj));
}
CloudProcessorP parseCloudProcessor(const QJsonObject &obj ){
    return CloudProcessorP(cloudProcessor_parser[obj["name"].toString().toLower().toStdString()](obj));
}

ClassifierConstructor* parseSVM (const QJsonObject & data){
    CvSVMParams params;
    params.term_crit.max_iter = data.contains("iterations") ? data["iterations"].toInt() : 1000;
    params.svm_type = CvSVM::C_SVC;
    auto type = data["type"].toString().toLower();
    if (type == "linear" ){
        params.kernel_type = CvSVM::LINEAR;
    }else if (type == "rbf"){
        params.kernel_type = CvSVM::RBF;
        params.gamma = data["gamma"].toDouble();
    }else if (type == "sigmoid"){
        params.kernel_type = CvSVM::SIGMOID;
        params.gamma = data["gamma"].toDouble();
        params.coef0 = data["coef0"].toDouble();
    }else if (type == "polynom"){
        params.kernel_type = CvSVM::POLY;
        params.gamma = data["gamma"].toDouble();
        params.coef0 = data["coef0"].toDouble();
        params.degree = data["degree"].toDouble();
    }
    return new SVMConstructor(params);
}


ClassifierConstrP parseClassifier(const QJsonObject & obj){
    return ClassifierConstrP(classifier_parser[obj["name"].toString().toLower().toStdString().c_str()](obj));
}


#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

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


ParseResult parseConfig(const string &filename){
    auto f = QString::fromStdString(filename);
    QFile file(f);
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    file.close();
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(data, & err).object();
    if (err.error != QJsonParseError::NoError){
        cout << "[FATAL] Error while parsing json: "<<err.errorString().toStdString() << endl;
        exit(EXIT_FAILURE);
    }
    ParseResult res;
    res.landmarkdir = json["landmark_dir"].toString().toStdString();
    res.actiondir = json["action_dir"].toString().toStdString();
    res.actionnames = parseArray(json["action_names"].toArray(),[](const QJsonValue & a){return a.toString().toStdString();});
    res.action_threshold = json["action_threshold"].toInt();
    res.training_percent = json["training_percent"].toDouble();
    res.classifier =  parseArray(json["classifier"].toArray(),arrayf(parseClassifier));
    res.cloud_processor =  parseArray(json["cloud_processor"].toArray(),arrayf(parseCloudProcessor));
    res.feature_processor =  parseArray(json["feature_processor"].toArray(),arrayf(parseFeatureProcessor));
    res.frame_features =  parseArray(json["frame_features"].toArray(),arrayf(parseFrameFeature));
    res.time_features =  parseArray(json["time_features"].toArray(),arrayf(parseTimeFeature));
    return res;
}


