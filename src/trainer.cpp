#include "classifier.h"
#include "featureextraction.h"
#include <iostream>
#include <string>
#include "reader.h"
#include "actionunit.h"
#include <chrono>

using namespace std;
using TruthList = std::vector<int8_t>;
using FeatureList = std::vector<cv::Mat>;

cv::Mat featureListToMat(const FeatureList & l){
    if (l.size() == 0) return cv::Mat();
    int cols =  l[0].cols;
    int rows = l.size();
    cv::Mat res(rows,cols,CV_32FC1);
    for(int i=0; i< rows; i++){
        l[i].copyTo(res.row(i));
    }
    return res;
}

cv::Mat truthListToMat(const TruthList& l){
    cv::Mat res(l.size(), 1,CV_32FC1);
    for (int i=0; i< l.size(); i++){
        res.at<float>(i,0) = l[i];
    }
    return res;
}

struct Args{
  vector<string> landmark_file;
  vector<string> action_unit_file;
  string feature_extract_method;
  string classifier_name;
  string output_classifier_filename;
  string action_unit_name;
  int intensity_threshold;
  float training_percent;
  string pca_filename;
  int pca_dim;
  int train_iterations;
  bool check_correctness(){
      return  landmark_file.size() == action_unit_file.size()
              && !action_unit_name.empty()
              && landmark_file.size() > 0
              && intensity_threshold >= 0&& intensity_threshold<=5
              && training_percent > 0 && training_percent <= 1
              && pca_dim >0 && pca_dim <= 66*2;

              /* ... */;
  }
};

inline string escape(const string &s){
    string res = s;
    string::size_type i=0;
    while(true){
        i=res.find("\"",i);
        if (i==string::npos) break;
        res.replace(i,1,"\\\"");
        i=i+1;
    }
    return res;
}

std::ostream& operator <<(std::ostream& stream, const vector<string>& v) {
   stream << "[" ;
   for (string j: v){
       stream << "\""<< escape(j)<<"\""<<",";
   }
   stream << "]";
   return stream;
}

std::ostream& operator <<(std::ostream& stream, const Args& args) {
   stream << "Landmark_Files "<< args.landmark_file <<", "
        << "ActionUnit_Files " << args.action_unit_file<<", "
        << "Feature_Method " << args.feature_extract_method<<", "
        << "Classfier "<<args.classifier_name<<", "
        << "Training-Save-Filename " << args.output_classifier_filename<<", "
        << "Action-Unit "<<args.action_unit_name<<", "
        << "Intensity-Threshold "<<args.intensity_threshold<<", "
        << "Training-Percent "<<args.training_percent<< ", "
        << "PCA data "<<args.pca_filename << ", "
        << "PCA dimension "<<args.pca_dim << ", "
        << "Training iterations "<<args.train_iterations<<endl ;
   return stream;
}

Args defaultArgs(){
    return Args{
        vector<string>(), vector<string>(),
        "SimpleNorm", "SVM",
        "", "", 2, 0.6f, "", 66*2, 10000
    };
}

unique_ptr<ClassifierConstructor> classifier_by_name(string name, Args args){
    if (name == "SVM") {
        CvSVMParams params;
        params.svm_type = CvSVM::C_SVC;
        params.kernel_type = CvSVM::RBF;
        params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER,args.train_iterations,1e-6);
        return unique_ptr<ClassifierConstructor>(new SVMConstructor(params));
    }
    return nullptr;
}

unique_ptr<FeatureExtractionBase<66>> extractor_by_name(string name, const Args  &args){
   //TODO: Imeplement;
   auto res = new SimpleNormalizeFeatureExtraction<66>(args.pca_dim);//unique_ptr<FeatureExtractionBase<66>>();
   if (!args.pca_filename.empty())
       res->set_pca(unique_ptr<PCA_Result<66>>(new PCA_Result<66>(PCA_Result<66>::load(args.pca_filename))));
   return unique_ptr<FeatureExtractionBase<66>>(res);
}

Args parseArgs(int argc, char** argv){
    Args res = defaultArgs();
    for (int i=0; i< argc; i+=2){
        string a(argv[i]);
        if (a=="-l") res.landmark_file.push_back(argv[i+1]);
        else if (a=="-a") res.action_unit_file.push_back(argv[i+1]);
        else if (a=="-o") res.output_classifier_filename = argv[i+1];
        else if (a=="-fm") res.feature_extract_method = argv[i+1];
        else if (a=="-cl") res.classifier_name = argv[i+1];
        else if (a=="-au") res.action_unit_name = argv[i+1];
        else if (a=="-it") res.intensity_threshold = atof(argv[i+1]);
        else if (a=="-tp") res.training_percent = atof(argv[i+1]);
        else if (a=="-pcafile") res.pca_filename = argv[i+1];
        else if (a=="-pcadim") res.pca_dim = atoi(argv[i+1]);
        else if (a=="-tit") res.pca_dim = atoi(argv[i+1]);
    }
    return res;
}

void printHelp(char* progName){
    cout << progName << endl
         <<  " -l       -  Add Landmark-File" <<endl
         <<  " -a       -  Add Action-Unit File" << endl
         <<  " -o       -  Filename for saving classifier result" << endl
         <<  " -fm      -  Method to extract features" << endl
          << " -cl      -  Classifier to use" << endl
          << " -au      -  Name for action unit to train for" << endl
          << " -it      -  Threshold for intensity" << endl
          << " -tp      -  Percent use for training" << endl
          << " -pcafile -  File to pca eigen file" << endl
          << " -tit     -  Trainings iterations" << endl;
}


#include <random>
std::vector<std::size_t> randomPermutation(std::size_t size){
    std::vector<std::size_t> res; res.reserve(size);
    for (int i=0; i< size; i++){
       res.push_back(i);
    }
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(res.begin(),res.end(),generator);
    return res;
}

bool train_evaluate_and_save(const Args &arg) {
    auto classifier_constructor = classifier_by_name(arg.classifier_name,arg);
    auto feature_extractor = extractor_by_name(arg.feature_extract_method, arg);

    // Load Points and Action Unit, then extract
    // desired action class
    vector<PointCloud<66>> total_set; TruthList total_truth;
    for(int i=0; i< arg.landmark_file.size(); i++){
       // Load Landmarks
       auto video = readBinaryFile(arg.landmark_file[i]);
       for(auto &&points: video){
           total_set.push_back(PointCloud<66>(std::move(points)));
       }
       // Load Action Units
       auto action_unit = readActionUnitFromFile(arg.action_unit_file[i]);
       // Get Index of desired action unit
       int action_idx = -1;
       for (int j=0; j< action_unit.getActionsAsName().size(); j++){
           if (action_unit.getActionsAsName()[j] == arg.action_unit_name){
               action_idx = j; break;
           }
       }
       if (action_idx < 0) {
           cout << "FATAL: Action Unit "<<arg.action_unit_name << " not found!" << endl;
           return false;
       }
       // Set action unit class as active, if desired action is higher-equal
       // than intensity_threshold
       for (int j=0; j < action_unit.size(); j++){
           int intensity = action_unit.getActionsIntensity(j+1)[action_idx];
           if (intensity < arg.intensity_threshold)
               total_truth.push_back(0);
           else total_truth.push_back(1);
       }
    }

    // Extract Features from Pointcloud
    FeatureList total_feature_set;
    for (auto && cloud: total_set){
        total_feature_set.push_back(feature_extractor->extractFeatures(cloud));
    }

    if (total_feature_set.size() != total_truth.size()){
        cout << "[FATAL] Landmarks and Actionunit does not have same size"<<endl;
        return false;
    }

    // Extract Training Set and Evaluation Set using random permutation
    int training_size = static_cast<int>(static_cast<float>(total_feature_set.size()) * arg.training_percent);
    cout << "[Info] Using "<<training_size << " from "<< total_feature_set.size() << " for training" << endl;

    auto perm = randomPermutation(total_feature_set.size());

    FeatureList training_set; TruthList training_truth;
    for (int i=0; i< training_size; i++){
        training_set.push_back(total_feature_set[perm[i]]);
        training_truth.push_back(total_truth[perm[i]]);
    }
    FeatureList eval_set; TruthList eval_truth;
    for (int i=training_size; i< total_feature_set.size(); i++){
        eval_set.push_back(total_feature_set[perm[i]]);
        eval_truth.push_back(total_truth[perm[i]]);
    }
    cout << "* Start training"<< endl;
    std::chrono::time_point<std::chrono::system_clock> start, end; // Time Measure
    start = std::chrono::system_clock::now();
    auto classifier = classifier_constructor->train(featureListToMat(training_set), truthListToMat(training_truth));
    end = std::chrono::system_clock::now();
    if (classifier == nullptr){
       cout << "FATAL: Training failed!" << endl;
       return false;
    }
    cout << "-> Traing finished ["<<chrono::duration_cast<chrono::milliseconds>((end-start)).count()<<"ms]" << endl;
    cout << "* Training Confusion-Matrix"<< endl;
    cout << computeConfusionMatrixFromTestSet(*classifier, featureListToMat(training_set), truthListToMat(training_truth)) << endl;
    if (!arg.output_classifier_filename.empty()){
        cout << "* Save classifier to "<<arg.output_classifier_filename;
        if (!classifier->serialize(arg.output_classifier_filename)){
        cout << "FATAL: Saving failed!" << endl;
        return false;
        }
        cout << "-> Saved sucessfully" << endl;
    }
    cout << "* Evaluate classifier" << endl;
    cout << "** Confusion-Matrix" << endl;
    auto conf_matrix = computeConfusionMatrixFromTestSet(*classifier, featureListToMat(eval_set), truthListToMat(eval_truth));
    cout << conf_matrix << endl;
    cout << "** Recall: " <<computeRecall(conf_matrix)<<endl;
    cout << "** Precision: " <<computePrecision(conf_matrix)<<endl;
    return true;
}

int main(int argc, char** argv){
    auto args = parseArgs(argc-1,argv+1);
    cout << "Arguments: "<< args<<endl;
    if (!args.check_correctness()){
        cout << "[FATAL] Invalid Arguments" << endl;
        printHelp(argv[0]);
        return EXIT_FAILURE;
    }
    if (train_evaluate_and_save(args)){
        return EXIT_SUCCESS;
    }else {
        return EXIT_FAILURE;
    }
}
