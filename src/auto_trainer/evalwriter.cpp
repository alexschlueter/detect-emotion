#include "evalwriter.h"
#include "featureextraction.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <map>
using namespace std;

template <typename T>
using SerialMap = map<string, std::function<QJsonObject(const T &)>>;

#define standard_entry(x,type,ename) {typeid(x).name(), [](const type & t){QJsonObject res; res["name"]=ename; return res;}}

#define standard_classifier_entry(x,name) standard_entry(x,ClassifierConstructor,name)
static SerialMap<ClassifierConstructor> classifier_serializer = {
    standard_classifier_entry(SVMConstructor, "svm"),
    standard_classifier_entry(RandomForestConstructor, "randomforest")
};
#undef standard_classifier_entry

QJsonObject serializeClassifier(const ClassifierConstructor & classifier){
    return classifier_serializer[typeid(classifier).name()](classifier);
}

#define standard_cloudp_entry(x,name) standard_entry(x,CloudProcessor,name)
static SerialMap<CloudProcessor> cloud_proc_serializer = {
     standard_cloudp_entry(CloudNormalizer,"PointCloudNormalization"),
     standard_cloudp_entry(PCACloudReducer,"PCAReducer"),
     {typeid(CloudMask).name(), [](const CloudProcessor & c){
        const CloudMask & cloudmask = dynamic_cast<const CloudMask&>(c);
        auto tokeep = cloudmask.tokeep();
        QJsonObject res;
          res["name"] = "mask";
          QJsonArray tokeepjs;
          for (int i: tokeep){ tokeepjs.push_back(i);}
          res["tokeep"] = tokeepjs;
          return res;
      }}
};
#undef standard_cloudp_entry

QJsonObject serializeCloudProcessor(const CloudProcessor & processor){
    return cloud_proc_serializer[typeid(processor).name()](processor);
}

#define standard_frame_extr_entry(x,name) standard_entry(x,FeatureExtractionBase<66>,name)
static SerialMap<FeatureExtractionBase<66>> frame_extr_serializer = {
    standard_frame_extr_entry(XYFeatureExtraction<66>, "xy"),
    standard_frame_extr_entry(InterpolationFeatureExtraction, "interpolation")
};
#undef standard_frame_extr_entry
QJsonObject serializeFrameFeature(const FeatureExtractionBase<66> & extractor){
    return frame_extr_serializer[typeid(extractor).name()](extractor);
}

static SerialMap<TimeFeatureExtractionBase<66>> time_extr_serializer = {
    {typeid(SimpleTimeDifferentialExtraction<66>).name(), [](const TimeFeatureExtractionBase<66> & extractor){
         auto timediff = dynamic_cast<const SimpleTimeDifferentialExtraction<66>&>(extractor);
        QJsonObject res;
         res["name"] = "differential";
         res["base"] = serializeFrameFeature(*timediff.base_feature());
         res["truth_threshold"] = timediff.truth_diff_threshold();
         return res;
     }}
};
QJsonObject serializeTimeFeature(const TimeFeatureExtractionBase<66> & extractor){
 return time_extr_serializer[typeid(extractor).name()](extractor);
}

#define standard_feature_proc_entry(x,name) standard_entry(x,FeatureProcessor,name)
static SerialMap<FeatureProcessor> featu_proc_serializer = {
    standard_feature_proc_entry(FeatureMinMaxNormalizer,"minmaxnormalize"),
    standard_feature_proc_entry(PCAFeatureReducer,"pcaReducer")
};
#undef standard_feature_proc_entry
QJsonObject serializeFeatureProcessor(const FeatureProcessor & proc){
 return featu_proc_serializer[typeid(proc).name()](proc);
}

QJsonArray serializeFeatureProcessorArray(const vector<FeatureProcessorP> & procv, const string & outdir){
        QJsonArray processors;
        for (auto && proc: procv){
            if (proc->onlyOnTrainingSet()) continue;
            auto proc_obj = serializeFeatureProcessor(*proc);
            proc_obj["load"] = QString::fromStdString(outdir+"/"+proc->name());
            processors.push_back(proc_obj);
        }
        return processors;
}
string serializeEvalConfig(
        const string & actionname,
        const string & cloud_save_dir,
        const vector<CloudProcessorP> & cloud_processors,
        const ClassifierConstructor & classifier,
        const string & classifier_save_filename,
        int action_threshold,
        const string & processor_save_dir,
        const FrameFeatureProcessor * frameFeature,
        const TimeFeatureProcessor * timeFeature,
        int time_frame_step){
    QJsonObject mainObj;
    mainObj["landmark_dir"] = QString();
    mainObj["action_dir"] = QString();

    QJsonArray action_names;
    action_names.push_back(QString::fromStdString(actionname));
    mainObj["action_names"] = action_names;

    mainObj["action_threshold"] = action_threshold;

    auto classifier_js = serializeClassifier(classifier);
    classifier_js["load"] = QString::fromStdString(classifier_save_filename);
    mainObj["classifier"] = classifier_js;

    QJsonArray cloud_proc_json;
    for (auto && proc : cloud_processors){
        if (proc->onlyOnTrainingSet()) continue;
        auto obj = serializeCloudProcessor(*proc);
        if (proc->serializable()){
            obj["load"] = QString::fromStdString(cloud_save_dir+"/"+proc->name());
        }
        cloud_proc_json.push_back(obj);
    }
    mainObj["cloud_processor"] = cloud_proc_json;

    if (timeFeature != nullptr){
        QJsonObject feature = serializeTimeFeature(*timeFeature->extractor);
        QJsonArray processors = serializeFeatureProcessorArray(timeFeature->processors,processor_save_dir);
        feature["processors"] = processors;
        mainObj["time_feature"] = feature;
        mainObj["time_frame_step"] = time_frame_step;
    }else if (frameFeature != nullptr){
        QJsonObject feature = serializeFrameFeature(*frameFeature->extractor);
        QJsonArray processors = serializeFeatureProcessorArray(frameFeature->processors,processor_save_dir);
        feature["processors"] = processors;
        mainObj["frame_feature"] = feature;
    }else{
        cerr << "[ERROR] Cannot serialize to json, no time- or framefeature"<<endl;
        return string();
    }

    QJsonDocument document;
    document.setObject(mainObj);
    return document.toJson().toStdString();
}
