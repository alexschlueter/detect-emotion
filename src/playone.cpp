#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <array>
#include <limits>
#include <chrono>
#include <thread>
#include "reader.h"
#include "utils.h"
#include "normalization.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <cmath>

#include "features.h"
#include "pointcloud.h"
#include "pcanalysis.h"
#include "actionunit.h"
#include "configuration.h"

using namespace std;

const float RESOLUTION = 2048;
const int FONT = cv::FONT_HERSHEY_SIMPLEX;
const bool PRINT_INDICES = false;


int main(int argc, char** argv) {
    Configuration config("config.cfg");
    auto landmarks = readBinaryFolder(config.getStringValue("landmark_folder", "./landmarks"));
    std::cout << "Total number of frames: " << landmarks.size() << std::endl;

    std::vector<PointCloud<66>> pointClouds(landmarks.size());
    for(int i=0; i<landmarks.size(); i++)
    {
        pointClouds[i] = PointCloud<66>(landmarks[i]);
    }
    standardizePointCloudVector<66>(pointClouds, RESOLUTION / 2.f);

    ActionUnit actionUnit = readActionUnitFromFolder(config.getStringValue("actionunit_folder", "./actionunits"));
    // copy selected feature into single col matrix
    cv::Mat labelSet(actionUnit.mat.rows - 1, 1, CV_32SC1);
    std::cout << "Label dimensions " << labelSet.rows << "x" << labelSet.cols << std::endl;

    /* Create Feature Extractors */
    cv::Mat img;
    FeatureExtractionAggregate<66> featureExtractor;
    // featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new EuclideanDistanceExtraction<66>()));
    // featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new OrientationExtraction<66>()));
    featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new CenterOrientationExtraction<66>()));
    featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new CenterDistanceExtraction<66>()));
    cv::Mat featureSet = extractFeaturesFromData<66>(pointClouds, &featureExtractor);

    /* Print */
    std::cout << landmarks.size() << " frames" << std::endl;
    std::cout << "Feature set is " << featureSet.rows << "x" << featureSet.cols << std::endl;

    std::cout << "Starting PCA..." << std::endl;
    PCAnalysis pca(featureSet, 0.95, true);
    pca.print();

    cv::Mat projected = pca.project(featureSet);
    std::cout << "Projection set is " << projected.rows << "x" << projected.cols << std::endl;

    cv::namedWindow("Image");
    for (int i=0; i<pointClouds.size(); i++) {
        auto frame = pointClouds[i];

        cv::Point2f imageSize(frame.greatestXDistance() * 2, frame.greatestYDistance() * 2);
        cv::Point2f quarterImageSize(imageSize.x / 2.f, imageSize.y / 2.f);

        int label = labelSet.at<int>(i, 0);
        // std::cout << "LABEL IS " << label << " AND PREDICTION IS " << prediction << std::endl;

        img = cv::Mat::zeros(imageSize.y, imageSize.x, CV_8UC3);


        for (int i = 0; i < 66; ++i) {

            if(PRINT_INDICES)
            {
                cv::putText(img, to_string(i), frame[i] + quarterImageSize, FONT, 1, cv::Scalar(255,255,255));
            }
            else
            {
                cv::circle(img, frame[i] + quarterImageSize, 3, cv::Scalar(255,255,255));
            }
        }

        cv::imshow("Image", img);
        if (cv::waitKey(50) == 'q') break;
    }
}
