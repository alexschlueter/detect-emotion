#include <iostream>
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

#include "features.h"
#include "PointCloud.h"
using namespace std;

const float RESOLUTION = 2048;
const int FONT = cv::FONT_HERSHEY_SIMPLEX;
const bool PRINT_INDICES = false;

int main(int argc, char** argv) {
    if (argc < 2){
        cout << argv[0] << " path-to-landmark.bin"<<endl;
        return EXIT_FAILURE;
    }
    ifstream lm1(argv[1], ios::binary);
    // lm1.ignore(numeric_limits<streamsize>::max(), '\n');
    std::vector<std::array<cv::Point2f, 66>> landmarks = readBinaryFile(lm1);
    if (landmarks.empty()) cout << "Landmarks could not be loaded.";
    std::vector<PointCloud<66>> pointClouds(landmarks.size());
    for(int i=0; i<landmarks.size(); i++)
    {
        pointClouds[i] = PointCloud<66>(landmarks[i]);
    }
    standardizePointCloudVector<66>(pointClouds, RESOLUTION / 2.f);


    /* Create Feature Extractors */
    FeatureExtractionAggregate<66> featureExtractor;
    featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new EuclideanDistanceExtraction<66>()));
    featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new OrientationExtraction<66>()));

    EuclideanDistanceExtraction<66> edExtractor;
    OrientationExtraction<66> orExtractor;

    cv::namedWindow("Image");
    for (auto frame : pointClouds) {
        cv::Point2f imageSize(frame.greatestXDistance() * 2, frame.greatestYDistance() * 2);
        cv::Point2f quarterImageSize(imageSize.x / 2.f, imageSize.y / 2.f);

        cv::Mat featuresX = edExtractor.extractFeatures(frame);
        cv::Mat featuresY = orExtractor.extractFeatures(frame);

        cv::Mat img = cv::Mat::zeros(imageSize.y, imageSize.x, CV_8UC3);

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

        /* Draw euclidean distance on X- and orientation on Y-Axis */
        // for(int i = 0; i < featuresX.size().height; i++)
        // {
        //     cv::circle(img, cv::Point2f(featuresX.at<float>(i), std::fabs(featuresY.at<float>(i)) * 300), 1, cv::Scalar(255,0,0));
        // }

        cv::imshow("Image", img);
        if (cv::waitKey(50) == 'q') break;
    }
}
