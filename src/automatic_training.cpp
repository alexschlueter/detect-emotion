#include "configuration.h"
#include "training_set.h"
#include "reader.h"
#include "actionunit.h"
#include "pcanalysis.h"
#include "featureextraction.h"
#include "classifier.h"
#include "featurescaling.h"
#include <memory>
#include <chrono>

/** Returns a vector of feature extractor combinations. */
std::vector<std::shared_ptr<FeatureExtractionBase<66>>> getFeatureExtractionSet()
{
    std::vector<std::shared_ptr<FeatureExtractionBase<66>>> result;

    /** 1) Create Feature extraction that keeps x and y coordinates. */
    result.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new XYFeatureExtraction<66>()));

    /** 2) Create Feature extraction with distance and orientation to point cloud center. */
    std::shared_ptr<FeatureExtractionAggregate<66>> centerRelationExtraction(new FeatureExtractionAggregate<66>());
    centerRelationExtraction->extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new CenterOrientationExtraction<66>()));
    centerRelationExtraction->extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new CenterDistanceExtraction<66>()));
    result.push_back(centerRelationExtraction);

    /** 3) Create feature extraction that only keeps meaningful landmarks for the action unit */
    // TODO: masks path from config file, au number from action_name config
    std::ifstream maskFile("../../masks/au25.txt");
    result.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new MaskFeatureExtraction<66>(maskFile)));

    return result;
}

std::vector<CvSVMParams> getSVMParamSet(int trainingIterations = 1000)
{
    std::vector<CvSVMParams> result;

    /** 1) Default parameter set. */
    CvSVMParams defaultParams;
    defaultParams.term_crit.max_iter = trainingIterations;
    result.push_back(defaultParams);

    // /** 2) Linear parameter set. */
    // CvSVMParams linearParams;
    // linearParams.svm_type = CvSVM::C_SVC;
    // linearParams.kernel_type = CvSVM::LINEAR;
    // linearParams.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, trainingIterations, 1e-6);
    // result.push_back(linearParams);

    return result;
}

int main(int argc, char** argv)
{
    Configuration config("configuration.cfg");

    /** Load all Landmarks from landmark dir */
    auto landmarks = readBinaryFolder(config.getStringValue("landmark_dir", "./landmarks"));
    std::cout << "Loaded " << landmarks.size() << " frames." << std::endl;

    /** Load all Action-Units from action unit dir */
    ActionUnit actionUnit = readActionUnitFromFolder(config.getStringValue("action_unit_dir", "./actionunits"));
    cv::Mat labels;
    if(!actionUnit.getActionByName(config.getStringValue("action_name", "Chin Raiser"), labels))
    {
        std::cout << "ERROR: Action with name '" << config.getStringValue("action_name", "Chin Raiser") << "' not found." << std::endl;
        return 0;
    }
    std::cout << "Label dimensions: " << labels.rows << "x" << labels.cols << std::endl;

    /** Create Pointclouds from Landmarks */
    std::vector<PointCloud<66>> pointClouds(landmarks.size());
    for(int i = 0; i < landmarks.size(); i++)
    {
        pointClouds[i] = PointCloud<66>(landmarks[i]);
    }
    standardizePointCloudVector<66>(pointClouds);


    int loopCount = 1;
    auto featureExtractionSet = getFeatureExtractionSet();
    for(auto extraction : featureExtractionSet)
    {
        std::cout << std::endl;

        /** Extract features */
        std::cout << "*** STARTING LOOP #" << loopCount << " ***" << std::endl;
        std::cout << "Extracting features..." << std::endl;
        cv::Mat features = extractFeaturesFromData<66>(pointClouds, *extraction);
        std::cout << "Feature dimensions " << features.rows << "x" << features.cols << std::endl;

        /** Split Training, Test, Validation data. */
        std::cout << "Creating Trainings-, Test- and Validation-Set." << std::endl;
        TrainingSet ttvSet(features, labels, config.getFloatValue("training_percent", 0.5), config.getFloatValue("test_percent", 0.25), config.getFloatValue("validation_percent", 0.25));
        std::cout << "Training set has " << ttvSet.trainingSize() << " + " << ttvSet.testSize() << " + " << ttvSet.validationSize() << " entries." << std::endl;
        ttvSet.shuffle();
        cv::Mat trainingFeatures = ttvSet.trainingFeatures();
        cv::Mat trainingLabels = ttvSet.trainingLabels(true);

        /** Create PCA only from training set */
        // float retainedVariance = config.getFloatValue("retained_variance", 0.95);
        // std::cout << "Creating PCA from training set with " << retainedVariance << " retained variance..." << std::endl;
        // PCAnalysis pca(trainingFeatures, retainedVariance, true);
        // std::cout << "Finished PCA. Retained " << pca.getNumComponents() << " principal components." << std::endl;

        /** Invoke PCA on training set */
        // std::cout << "Invoking PCA projection on training set..." << std::endl;
        // trainingFeatures = pca.project(trainingFeatures);

        /** Memorize min, max values in training set and scale features to [0, 1] */
        FeatureScaling featScale(trainingFeatures);

        /** Create and train SVM with different parameter sets. */
        auto parameterSets = getSVMParamSet(config.getIntValue("training_iterations", 10000));
        int innerLoopCount = 1;
        for(auto& params : parameterSets)
        {
            std::unique_ptr<ClassifierConstructor> classifierConstructor(new SVMConstructor(params));

            std::cout << std::endl << "\t*** Training SVM with parameter set #" << innerLoopCount << " ***" << std::endl;
            auto classifier = classifierConstructor->train(trainingFeatures, trainingLabels);

            std::cout << "\tEvaluating on test set..." << std::endl;
            // cv::Mat testFeatures = pca.project(ttvSet.testFeatures());
            cv::Mat testFeatures = ttvSet.testFeatures();
            cv::Mat testLabels = ttvSet.testLabels(true);

            /** Scale test features using the memorized min / max values from training set */
            featScale.scale(testFeatures);

            /** Calculate precision manually... */
            int error = 0, success = 0;
            for(int i=0; i<testFeatures.rows; i++)
            {
                int classification = classifier->classify(testFeatures.row(i));
                if(classification != testLabels.at<uint8_t>(i, 0))
                    error++;
                else
                    success++;
            }
            float precision = float(success) / (error + success);
            std::cout << "\tClassified " << success << "/" << (error + success) << " samples correctly, for a total precision of " << precision << std::endl;

            /** Calculate precision with confusion matrix... */
            std::cout << "\tComputing confusion matrix..." << std::endl;
            auto confusionMatrix = computeConfusionMatrixFromTestSet(*classifier, testFeatures, testLabels);

            // std::cout << "\t" << confusionMatrix << std::endl;
            std::cout << "\t\t Recall: " << computeRecall(confusionMatrix) << std::endl;
            std::cout << "\t\t Precision: " << computePrecision(confusionMatrix) << std::endl;


            innerLoopCount++;
        }
        loopCount++;
    }
    return 0;
}
