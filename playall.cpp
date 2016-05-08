#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <limits>
#include <string>

#include "opencv2/opencv.hpp"

using namespace std;

int main() {
    string path = "../Landmark_Points_bin/landmarks00";
    typedef vector<array<cv::Point2f, 66>> VideoT;
    array<VideoT, 10> in;
    for (int k = 0; k < 10; ++k) {
        cout << k << endl;
        if (k == 9) path.erase(path.end() - 1);
        ifstream lm(path + to_string(k + 1) + ".bin", ios::binary);
        if (! lm.is_open()) cout << "ups: " << k + 1;
        for (int j = 0; ! lm.eof(); ++j) {
            in[k].push_back({});
            for (int i = 0; i < 66; ++i) {
                lm.read((char*)&in[k][j][i].x, sizeof(float));
                lm.read((char*)&in[k][j][i].y, sizeof(float));
                // cout << j << " " << i << " ";
                // cout << in[j][i] << endl;
            }
        }
    }

    cout << "loaded!" << endl;

    array<VideoT::iterator, 10> where;
    for (int i = 0; i < 10; ++i) {
        where[i] = in[i].begin();
    }

    cv::namedWindow("Image", CV_WINDOW_NORMAL);
    const int vidH = 768;
    const int vidW = 1024;
    cv::Mat all = cv::Mat(2 * vidH, 5 * vidW, CV_8UC1);
    for (;;) {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 5; ++j) {
                cv::Mat thisimg = cv::Mat(all, {i * vidH, (i + 1) * vidH}, {j * vidW, (j + 1) * vidW});
                thisimg = cv::Mat::zeros(vidH, vidW, CV_8UC1);

                for (int k = 0; k < 66; ++k) {
                    // img.at<uchar>(frame[i])=255;
                    cv::Point2f center = (*where[(i + 1) * (j + 1) - 1])[k];
                    // center.x *= 384.0 / 1024.0;
                    // center.y *= 500.0 / 768.0;
                    cv::circle(thisimg, center, 3, {100, 100, 100});
                    // cv::putText(img, to_string((int)frame[i].x) + ", " + to_string((int)frame[i].y), frame[i], cv::FONT_HERSHEY_SIMPLEX, 0.3, {100, 100, 100});
                }
                where[(i + 1) * (j + 1) - 1]++;
            }
        }
        cv::imshow("Image", all);
        cv::waitKey(50);
    }
}
