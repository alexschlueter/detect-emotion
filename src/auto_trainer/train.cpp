#include "train.h"
#include <iostream>
#include <tuple>
#include "reader.h"
#include "actionunit.h"
#include "trainparser.h"
#include "util.h"
#include "wrapper.h"
#include "exporter.h"
#include "evalwriter.h"
using namespace std;


struct Trainer{
public:
    Trainer(TrainParseResult&& r_,const string & outpath_): r(std::move(r_)),outpath(outpath_){ }
    TrainParseResult r;
    string outpath;
    string cloud_out_dir;
    void loop();
    std::unique_ptr<PythonExport> exporter;
private:
    void loop_action_feature(const FeatureTruth & train, const FeatureTruth & test, const string & actionname, const string & extractorname, const vector<FeatureProcessorP> & processors, FrameFeatureProcessor* used_frame_feature,TimeFeatureProcessor * used_time_feature);
};

#if onunix

static Trainer * global_trainer = nullptr;

// Ctrl+C Handler
void sign_handler(int _){
    if (global_trainer != nullptr && global_trainer->exporter != nullptr){
        global_trainer->exporter->save();
    }
    exit(EXIT_FAILURE);
}
#endif

int trainmain(char* name, int argc, char** argv){
    if (argc < 1){
        cerr << "[ERROR] " << name << " train : path to configfile required"<<endl;
        return EXIT_FAILURE;
    }
    string filename(argv[0]);
    Trainer tr{parseTrainConfig(filename),dirOfFile(filename)};
#if onunix
    //Ctrl+C Handler
    global_trainer = &tr;
    signal(SIGINT, sign_handler);
#endif
    tr.loop();
    return EXIT_SUCCESS;
}

void Trainer::loop(){
    // Load data & build train and test set
    cout << "Loading data ..." << endl;
    CloudAction all_data{loadLandmarkFromFolder(r.landmarkdir), loadActionUnitFormFolder(r.actiondir)};
    if (all_data._landmarks.size() != all_data._actionUnits.size()){
        cerr<< "[ERROR] "<< "Landmark-Video-Size ("<<all_data._landmarks.size()<< ") and Action-Unit-Video-Size ("<<all_data._actionUnits.size()<<") does not match!"<< endl;
        return ;
    }
    for (int i=0; i < all_data.size(); i++){
        auto lsize = all_data._landmarks[i].size();
        auto asize = all_data._actionUnits[i].size();
        if (lsize != asize){
            cerr << "[ERROR] Video "<<i+1<<": Number of Landmarks-Frames ("<<lsize<<") and number of Action-Unit-Frames ("<<asize <<") does not match!"<<endl;
            return;
        }
    }
    cout << "Building Trainingset & Testset..." << endl;
    auto train_set = all_data.subset(0, all_data.size()*r.training_percent);
    cout << "- Trainset contains "<<train_set.size()<<" videos"<<endl;
    auto test_set = all_data.subset(all_data.size()*r.training_percent, all_data.size());
    cout << "- Testset contains "<<test_set.size()<<" videos"<<endl;

    cout << "Transform PointCloud..."<< endl;
    // Apply functions to PointCloud
    for (auto&& proc: r.cloud_processor){
       cout << " - Applying "<< proc->name()<< endl;
       proc->analyse(train_set);
       train_set = proc->apply(train_set);
       if (!proc->onlyOnTrainingSet())
         test_set = proc->apply(test_set);
    }

    // Save processors
    string savepath = outpath+"/processor_data/";
    this->cloud_out_dir = savepath;
    mkdirs(savepath);
    for(auto && proc: r.cloud_processor){
        proc->save(savepath+proc->name());
    }


    // Train with Action / Features
    for (auto && action: r.actionnames){
        exporter = unique_ptr<PythonExport>(new PythonExport(outpath+"/"+action+"/result.py"));
        cout << endl << "Train for Action "<< action<<endl;
        for (auto && feature_proc: r.frame_features_processors){
            const FeatureExtractionBase<66>*  feature = &*feature_proc.extractor;
            cout << "\tUsing frame-feature "<<feature->name()<<endl;
          loop_action_feature(train_set.extractFrameFeature(*feature,action,r.action_threshold),
                              test_set.extractFrameFeature(*feature,action,r.action_threshold),
                              action, feature->name(), feature_proc.processors,
                              &feature_proc,nullptr);
        }

        for (auto && feature_proc: r.time_features_processors){
            const TimeFeatureExtractionBase<66>*  feature = &*feature_proc.extractor;
            cout << "\tUsing time-feature "<<feature->name()<<endl;
            loop_action_feature(train_set.extractTimeFeature(*feature,action,r.action_threshold,r.time_frame_step),
                                test_set.extractTimeFeature(*feature,action,r.action_threshold, r.time_frame_step),
                                action, feature->name(),feature_proc.processors,
                                nullptr, &feature_proc);
        }
        exporter->save();
    }
}

void Trainer::loop_action_feature(const FeatureTruth & train, const FeatureTruth & test, const string & actionname, const string & extractorname, const vector<FeatureProcessorP> & processors, FrameFeatureProcessor * used_frame_feature,TimeFeatureProcessor* used_time_feature){
    FeatureTruth trainset = train;
    FeatureTruth testset = test;
    auto pos_train_size = trainset.positiveSamples().size();
    cout << "\t\tTrainset contains "<<pos_train_size<<" positives of "<<trainset.size()<<" features"<<endl;
    cout << "\t\tTestset contains "<<testset.positiveSamples().size()<<" positives of "<<testset.size()<<" features"<<endl;
    if (pos_train_size == 0 || pos_train_size == trainset.size()) {
        cout << "Train-Set has an invalid number of positives... skip"<<endl;
        return;
    }

    // Apply functions to features
    cout <<endl<<"\t\tTransform fetaures..."<< endl;
    for (auto && proc: processors){
        cout << "\t\t - Applying "<< proc->name()<< endl;
        proc->analyse(trainset);
        trainset = proc->apply(trainset);
        if (!proc->onlyOnTrainingSet())
          testset = proc->apply(testset);
    }

    // Save processors
    string curdir = outpath+"/"+actionname+"/"+extractorname;
    string savepath = curdir+"/data/";
    mkdirs(savepath);
    for(auto && proc: processors){
        proc->save(savepath+proc->name());
    }

    // Train every classifier
    for(auto && classifier_constr: r.classifier){
        cout << "\t\t\t - Train Classificator"<< endl;
        auto classifier = classifier_constr->train(trainset._features,trainset._truth);
        string classifier_outpath = savepath+"/classifier_"+classifier->name()+".dat";
        classifier->serialize(classifier_outpath);
        auto conf_train = computeConfusionMatrixFromTestSet(*classifier,trainset._features,trainset._truth);
        auto conf_test = computeConfusionMatrixFromTestSet(*classifier,testset._features,testset._truth);
        auto writeConfusion = [&classifier](const ConfusionMatrix& conf, const string & filename){
          ofstream stream(filename);
          double recall = computeRecall(conf);
          double precision = computePrecision(conf);
          stream << conf << endl;
          stream << "-------" << endl;
          stream << "Recall: "<<recall<<endl;
          stream << "Precision: "<<precision<<endl;
          stream.close();
          return make_tuple(recall, precision);
        };
        string name = extractorname+"-"+classifier->name();
        auto train_res = writeConfusion(conf_train,curdir+"/"+classifier->name()+"_train_result.txt");
        exporter->addTrainResult(get<0>(train_res),get<1>(train_res),name);
        auto test_res = writeConfusion(conf_test,curdir+"/"+classifier->name()+"_test_result.txt");
        exporter->addTestResult(get<0>(test_res),get<1>(test_res),name);

        string evalconfig = serializeEvalConfig(
                    actionname, relativeTo(curdir,cloud_out_dir),
                    r.cloud_processor,*classifier_constr,
                    relativeTo(curdir,classifier_outpath),r.action_threshold,
                    relativeTo(curdir,savepath),used_frame_feature,used_time_feature,
                    r.time_frame_step);
        ofstream stream(curdir+"/config_"+classifier->name()+".json");
        stream << evalconfig;
        stream.close();
    }
}
