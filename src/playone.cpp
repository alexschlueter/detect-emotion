#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <limits>
#include <chrono>
#include <thread>
#include "reader.h"
#include "utils.h"

#include "opencv2/opencv.hpp"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2){
        cout << argv[0] << " path-to-landmark.bin"<<endl;
        return EXIT_FAILURE;
    } 
    ifstream lm1(argv[1], ios::binary);
    // lm1.ignore(numeric_limits<streamsize>::max(), '\n');
    auto landmarks = readBinaryFile(lm1);
    if (landmarks.empty()) cout << "ups";

    cv::namedWindow("Image");
    for (auto frame : landmarks) {
        auto bestSize = findMaxInArray<cv::Point2f,66>(frame);
        // cv::Mat img = cv::Mat::zeros(768, 1024, CV_8UC1);
        cv::Mat img = cv::Mat::zeros(bestSize.y+20, bestSize.x+20, CV_8UC1);
        for (int i = 0; i < 66; ++i) {
            // img.at<uchar>(frame[i])=255;
            cv::circle(img, frame[i], 3, 255);
            // cv::putText(img, to_string((int)frame[i].x) + ", " + to_string((int)frame[i].y), frame[i], cv::FONT_HERSHEY_SIMPLEX, 0.3, {100, 100, 100});
        }
        cv::imshow("Image", img);
        if (cv::waitKey(50) == 'q') break;
    }
}
