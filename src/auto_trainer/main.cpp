#include <iostream>
#include "reader.h"
#include "actionunit.h"
#include "trainparser.h"
#include "util.h"
#include "wrapper.h"
using namespace std;

struct Trainer{
public:
    ParseResult r;
    string outpath;
    void loop();
private:
    void loop_action_feature(const FeatureTruth & train, const FeatureTruth & test, const string& actionname, const string & extractorname);
};


int main(int argc, char** argv){
    if (argc < 2){
        cout << argv[0] << "configfile"<<endl;
        return EXIT_FAILURE;
    }
    string filename(argv[1]);
    Trainer tr{parseConfig(filename),dirOfFile(filename)};
    tr.loop();
    return EXIT_SUCCESS;
}

void Trainer::loop(){
    // Load data & build train and test set
    cout << "Loading data ..." << endl;
    auto all_data = CloudAction{loadLandmarkFromFolder(r.landmarkdir), loadActionUnitFormFolder(r.actiondir)};
    cout << "Building Trainingset & Testset..." << endl;
    auto train_set = all_data.subset(0, all_data.size()*r.training_percent);
    auto test_set = all_data.subset(all_data.size()*r.training_percent, all_data.size());

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
    mkdirs(savepath);
    for(auto && proc: r.cloud_processor){
        proc->save(savepath+proc->name());
    }

    // Train with Action / Features
    for (auto && action: r.actionnames){
        cout << endl << "Train for Action "<< action<<endl;
        for (auto && feature: r.frame_features){
            cout << "\tUsing frame-feature "<<feature->name()<<endl;
          loop_action_feature(train_set.extractFrameFeature(*feature,action,r.action_threshold),
                              test_set.extractFrameFeature(*feature,action,r.action_threshold), action, feature->name());
        }

        for (auto && feature: r.time_features){
            cout << "\tUsing time-feature "<<feature->name()<<endl;
            loop_action_feature(train_set.extractTimeFeature(*feature,action,r.action_threshold),
                                     test_set.extractTimeFeature(*feature,action,r.action_threshold), action, feature->name());
        }
    }
}

void Trainer::loop_action_feature(const FeatureTruth & train, const FeatureTruth & test, const string & actionname, const string & extractorname){
    FeatureTruth trainset = train;
    FeatureTruth testset = test;
    cout << "\t\tTrainset contains "<<trainset.positiveSamples().size()<<" positives from "<<trainset.size()<<" features"<<endl;
    cout << "\t\tTestset contains "<<testset.positiveSamples().size()<<" positives from "<<testset.size()<<" features"<<endl;

    // Apply functions to features
    cout <<endl<<"\t\tTransform fetaures..."<< endl;
    for (auto && proc: r.feature_processor){
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
    for(auto && proc: r.feature_processor){
        proc->save(savepath+proc->name());
    }

    // Train every classifier
    for(auto && classifier_constr: r.classifier){
        cout << "\t\t\t - Train Classificator"<< endl;
        auto classifier = classifier_constr->train(trainset._features,trainset._truth);
        classifier->serialize(savepath+"/classifier_"+classifier->name()+".dat");
        auto conf_train = computeConfusionMatrixFromTestSet(*classifier,trainset._features,trainset._truth);
        auto conf_test = computeConfusionMatrixFromTestSet(*classifier,testset._features,testset._truth);
        auto writeConfusion = [&classifier](const ConfusionMatrix& conf, const string & filename){
          ofstream stream(filename);
          stream << conf << endl;
          stream << "-------" << endl;
          stream << "Recall: "<<computeRecall(conf)<<endl;
          stream << "Precision: "<<computePrecision(conf)<<endl;
          stream.close();
        };
        writeConfusion(conf_train,curdir+"/"+classifier->name()+"train_result.txt");
        writeConfusion(conf_test,curdir+"/"+classifier->name()+"test_result.txt");
    }
}
