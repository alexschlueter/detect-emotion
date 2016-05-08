#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <limits>
#include <chrono>
#include <thread>

#include "opencv2/opencv.hpp"

using namespace std;

int main() {
    ifstream lm1("../Landmark_Points_bin/landmarks001.bin", ios::binary);
    vector<array<cv::Point2f, 66>> in;
    if (! lm1.is_open()) cout << "ups";
    // lm1.ignore(numeric_limits<streamsize>::max(), '\n');
    for (int j = 0; ! lm1.eof(); ++j) {
        in.push_back({});
        for (int i = 0; i < 66; ++i) {
            lm1.read((char*)&in[j][i].x, sizeof(float));
            lm1.read((char*)&in[j][i].y, sizeof(float));
            // cout << j << " " << i << " ";
            // cout << in[j][i] << endl;
        }
    }

    cv::namedWindow("Image");
    for (auto frame : in) {
        cv::Mat img = cv::Mat::zeros(768, 1024, CV_8UC1);
        for (int i = 0; i < 66; ++i) {
            // img.at<uchar>(frame[i])=255;
            cv::circle(img, frame[i], 3, {100, 100, 100});
            // cv::putText(img, to_string((int)frame[i].x) + ", " + to_string((int)frame[i].y), frame[i], cv::FONT_HERSHEY_SIMPLEX, 0.3, {100, 100, 100});
        }
        cv::imshow("Image", img);
        cv::waitKey(50);
    }
}
