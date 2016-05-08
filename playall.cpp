#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>

#include "opencv2/opencv.hpp"

using namespace std;

void onTrackbarChange(int pos, void *userdata) {
    *(int*)userdata = pos;
}

int main() {
    string path = "../Landmark_Points_bin/landmarks00";
    typedef vector<array<cv::Point2f, 66>> VideoT;
    array<VideoT, 10> in;
    int maxFrames = 0;
    for (int k = 0; k < 10; ++k) {
        cout << k << endl;
        if (k == 9) path.erase(path.end() - 1);
        ifstream lm(path + to_string(k + 1) + ".bin", ios::binary);
        if (! lm.is_open()) cout << "ups: " << k + 1;
        int j;
        for (j = 0; ! lm.eof(); ++j) {
            in[k].push_back({});
            for (int i = 0; i < 66; ++i) {
                lm.read((char*)&in[k][j][i].x, sizeof(float));
                lm.read((char*)&in[k][j][i].y, sizeof(float));
            }
        }
        if (j > maxFrames) maxFrames = j;
    }

    cout << "loaded!" << endl;

    int f = 0;
    int selected = -1;
    bool lmNums = false;
    bool paused = false;

    cv::namedWindow("Image", CV_WINDOW_NORMAL);
    cv::createTrackbar("Frame", "Image", nullptr, maxFrames - 1, onTrackbarChange, &f);
    const int vidH = 768;
    const int vidW = 1024;
    cv::Mat all = cv::Mat(2 * vidH, 5 * vidW, CV_8UC1);
    for (;;) {
        cv::setTrackbarPos("Frame", "Image", f);
        if (selected == -1) {
            all = cv::Mat::zeros(2 * vidH, 5 * vidW, CV_8UC1);
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 5; ++j) {
                    const int idx = (i + 1) * (j + 1) - 1;
                    if (f >= in[idx].size()) break;

                    cv::Mat thisimg = cv::Mat(all, {i * vidH, (i + 1) * vidH}, {j * vidW, (j + 1) * vidW});

                    cv::putText(thisimg, to_string(idx), {50, 50}, cv::FONT_HERSHEY_SIMPLEX, 1, {100, 100, 100});
                    for (int k = 0; k < 66; ++k) {
                        cv::Point2f center = in[idx][f][k];
                        cv::circle(thisimg, center, 3, {100, 100, 100});
                    }
                }
            }
            cv::imshow("Image", all);
        } else if (f < in[selected].size()) { // selected != -1
            cv::Mat thisimg = cv::Mat::zeros(vidH, vidW, CV_8UC1);

            cv::putText(thisimg, to_string(selected), {50, 50}, cv::FONT_HERSHEY_SIMPLEX, 1, {100, 100, 100});
            for (int k = 0; k < 66; ++k) {
                cv::Point2f center = in[selected][f][k];
                cv::circle(thisimg, center, 3, {100, 100, 100});
                if (lmNums) {
                cv::putText(thisimg, to_string(k), cv::Point2f(center.x + 2, center.y - 2), cv::FONT_HERSHEY_SIMPLEX, 0.3, {100, 100, 100});
                }
            }
            cv::imshow("Image", thisimg);
        }
        int k = cv::waitKey(50);
        if (k >= 48 && k <= 57) selected = k - 48;
        else if (k == 110) lmNums = ! lmNums;
        else if (k == 65361) f -= 1;
        else if (k == 65363) f += 1;
        else if (k == 32) paused = ! paused;
        else if (k != -1) selected = -1;

        if (! paused) ++f;
    }
}
