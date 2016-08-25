#include <iostream>
#include <tuple>
#include "reader.h"
#include "actionunit.h"
#include "trainparser.h"
#include "util.h"
#include "wrapper.h"
using namespace std;

struct PythonExport{
    PythonExport(string filename);
    void addTestResult(double recall, double precision, const string & name);
    void addTrainResult(double recall, double precision, const string & name);
    void writeResult();
    void writePlot();
    inline string text() const { return python_text.str();}
    stringstream python_text;
    stringstream test_res;
    stringstream train_res;
    inline void save(const string & filename){
        writeResult();
        writePlot();
        string text = this->text();
        ofstream files(filename);
        files << text;
        files.close();
    }
    inline void save(){
        save(_filename);
    }

    string _filename;
};

struct Trainer{
public:
    Trainer(ParseResult&& r_,const string & outpath_): r(std::move(r_)),outpath(outpath_){ }
    ParseResult r;
    string outpath;
    void loop();
    std::unique_ptr<PythonExport> exporter;
private:
    void loop_action_feature(const FeatureTruth & train, const FeatureTruth & test, const string& actionname, const string & extractorname, const vector<FeatureProcessorP>& processors);
};

PythonExport::PythonExport(string filename):_filename(filename){
    python_text << "import numpy as np\nfrom matplotlib import pyplot as plt" << endl << endl;
    test_res << "test_res = {" ;
    train_res << "train_res = {";
}

void PythonExport::writeResult(){
    test_res << "\n}";
    train_res << "\n}";
    python_text << test_res.str()<<endl<<endl<<train_res.str()<<endl;
}

#define checkInvalid(n) if (n == numeric_limits<double>::infinity() || isnan(n)) n = 0

void PythonExport::addTestResult(double recall, double precision, const string & name){
   checkInvalid(recall);
   checkInvalid(precision);
   test_res<<"\n\t\""<<name<<"\": {\"precision\":"<<precision<<", \"recall\": "<<recall<<"},";
}

void PythonExport::addTrainResult(double recall, double precision, const string & name){
   checkInvalid(recall);
   checkInvalid(precision);
   train_res<<"\n\t\""<<name<<"\": {\"precision\":"<<precision<<", \"recall\": "<<recall<<"},";
}


void PythonExport::writePlot(){
    python_text << "def plotIt(tset, name, ax):\n";
    python_text << "\tprecisions = np.array([tset[k]['precision'] for k in tset])\n";
    python_text << "\trecalls = np.array([tset[k]['recall'] for k in tset])\n";
    python_text << "\tlabels = tset.keys()\n";
    python_text << "\tax.plot(precisions, recalls,'pr')\n";
    python_text << "\tfor i in xrange(len(labels)):\n";
    python_text << "\t\tp,r = precisions[i], recalls[i]\n";
    python_text << "\t\tax.annotate(labels[i],"\
                     "xy=(p,r), xytext=(p,r+0.05), "\
                     "arrowprops=dict(facecolor='black', shrink=0.005), )\n";
    python_text << "\tax.set_ylim(0,1.02)\n";
    python_text << "\tax.set_xlim(0,1)\n";
    python_text << "\tax.set_title(name)\n";
    python_text << "\tax.set_xlabel('precision')\n";
    python_text << "\tax.set_ylabel('recall')\n";
    python_text << "fig = plt.figure()\n";
    python_text << "ax = fig.add_subplot(211)\n";
    python_text << "plotIt(test_res, 'test-result',ax)\n";
    python_text << "ax = fig.add_subplot(212)\n";
    python_text << "plotIt(train_res, 'train-result',ax)\n";
    python_text << "plt.show()\n";
}

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

int main(int argc, char** argv){
    if (argc < 2){
        cerr << "[ERROR] " << argv[0] << ": path to configfile required"<<endl;
        return EXIT_FAILURE;
    }
    string filename(argv[1]);
    Trainer tr{parseConfig(filename),dirOfFile(filename)};
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
        exporter = unique_ptr<PythonExport>(new PythonExport(outpath+"/"+action+"/result.py"));
        cout << endl << "Train for Action "<< action<<endl;
        for (auto && feature_proc: r.frame_features_processors){
            const FeatureExtractionBase<66>*  feature = &*feature_proc.extractor;
            cout << "\tUsing frame-feature "<<feature->name()<<endl;
          loop_action_feature(train_set.extractFrameFeature(*feature,action,r.action_threshold),
                              test_set.extractFrameFeature(*feature,action,r.action_threshold), action, feature->name(), feature_proc.processors);
        }

        for (auto && feature_proc: r.time_features_processors){
            const TimeFeatureExtractionBase<66>*  feature = &*feature_proc.extractor;
            cout << "\tUsing time-feature "<<feature->name()<<endl;
            loop_action_feature(train_set.extractTimeFeature(*feature,action,r.action_threshold,r.time_frame_step),
                                     test_set.extractTimeFeature(*feature,action,r.action_threshold, r.time_frame_step), action, feature->name(),feature_proc.processors);
        }
        exporter->save();
    }
}

void Trainer::loop_action_feature(const FeatureTruth & train, const FeatureTruth & test, const string & actionname, const string & extractorname, const vector<FeatureProcessorP> & processors){
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
        classifier->serialize(savepath+"/classifier_"+classifier->name()+".dat");
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
        auto train_res = writeConfusion(conf_train,curdir+"/"+classifier->name()+"train_result.txt");
        exporter->addTrainResult(get<0>(train_res),get<1>(train_res),name);
        auto test_res = writeConfusion(conf_test,curdir+"/"+classifier->name()+"test_result.txt");
        exporter->addTestResult(get<0>(test_res),get<1>(test_res),name);
    }
}
