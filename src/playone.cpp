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

    standardizePointCloudVector<66>(landmarks, 1024);
    cv::namedWindow("Image");
    for (auto frame : landmarks) {
        auto imageSize = calculateImageSize<66>(landmarks) * 2;
        // cv::Mat img = cv::Mat::zeros(768, 1024, CV_8UC1);
        cv::Mat img = cv::Mat::zeros(imageSize.y, imageSize.x, CV_8UC1);
        for (int i = 0; i < 66; ++i) {
            // img.at<uchar>(frame[i])=255;

            if(PRINT_INDICES)
            {
                cv::putText(img, to_string(i), frame[i] + imageSize / 4, FONT, 1, {255,255,255});
            }
            else
            {
                cv::circle(img, frame[i] + imageSize / 4, 3, 255);
            }
            // cv::putText(img, to_string((int)frame[i].x) + ", " + to_string((int)frame[i].y), frame[i], cv::FONT_HERSHEY_SIMPLEX, 0.3, {100, 100, 100});
        }
        cv::imshow("Image", img);
        if (cv::waitKey(50) == 'q') break;
    }
}
