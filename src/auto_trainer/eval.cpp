#include "eval.h"
#include "trainparser.h"
void evalFeature(const FeatureTruth & eval, const vector<FeatureProcessorP> & processors, const ClassifierP & classifier);

int evalmain(char *name, int argc, char **argv){
    if (argc < 1){
        cerr << "[ERROR] "<<name << "evaluate: path to configfile required"<<endl;
        return EXIT_FAILURE;
    }
    string filename(argv[0]);
    EvalParseResult evalconfig = parseEvalConfig(filename);
    CloudAction evalcloud{loadLandmarkFromFolder(evalconfig.landmarkdir), loadActionUnitFormFolder(evalconfig.actiondir)};
    if (evalcloud._landmarks.size() != evalcloud._actionUnits.size()){
        cerr<< "[ERROR] "<< "Landmark-Video-Size ("<<evalcloud._landmarks.size()<< ") and Action-Unit-Video-Size ("<<evalcloud._actionUnits.size()<<") does not match!"<< endl;
        return EXIT_FAILURE;
    }
    for (int i=0; i < evalcloud.size(); i++){
        auto lsize = evalcloud._landmarks[i].size();
        auto asize = evalcloud._actionUnits[i].size();
        if (lsize != asize){
            cerr << "[ERROR] Video "<<i+1<<": Number of Landmarks-Frames ("<<lsize<<") and number of Action-Unit-Frames ("<<asize <<") does not match!"<<endl;
            return EXIT_FAILURE;
        }
    }
    cout << "Transform PointCloud ..."<<endl;
    for (auto && proc: evalconfig.cloud_processor){
        if (!proc->onlyOnTrainingSet()){
            cout << " - Applying "<<proc->name() << endl;
            evalcloud = proc->apply(evalcloud);
        }
    }

    auto&& classifier = evalconfig.classifier;

    //string outpath = dirOfFile(filename);
    for (auto && action: evalconfig.actionnames){
        cout << "Eval Action-Unit "<<action<<endl;
        if (evalconfig.kind == EvalParseResult::Kind::frame){
            auto && extractor = evalconfig.frameFeature.extractor;
            auto && processors = evalconfig.frameFeature.processors;
            int threshold = evalconfig.action_threshold;
            auto feature = evalcloud.extractFrameFeature(*extractor,action,threshold);
            evalFeature(feature, processors, classifier);
        }else{
            auto && extractor = evalconfig.timeFeature.extractor;
            auto && processors = evalconfig.timeFeature.processors;
            int time_step = evalconfig.time_frame_step;
            int threshold = evalconfig.action_threshold;
            auto feature = evalcloud.extractTimeFeature(*extractor,action,threshold,time_step);
            evalFeature(feature,processors, classifier);
        }
    }
    return EXIT_SUCCESS;
}

void evalFeature(const FeatureTruth & eval, const vector<FeatureProcessorP> & processors, const ClassifierP & classifier){
    cout << "\tWe have "<<eval.positiveSamples().size() << " positives of "<<eval.size() << " features"<<endl;
    FeatureTruth evalfeatures = eval;
    for (auto && proc:processors){
        if(!proc->onlyOnTrainingSet()){
           cout << "\t - Applying "<<proc->name() << endl;
           evalfeatures = proc->apply(evalfeatures);
        }
    }
    auto confusion = computeConfusionMatrixFromTestSet(*classifier,evalfeatures._features,evalfeatures._truth);
    cout << "\t Confusion:"<< endl;
    cout << confusion << endl;
    cout << "\t-> Recall: "<<computeRecall(confusion)<<endl;
    cout << "\t-> Precision: "<<computePrecision(confusion)<<endl;
}
