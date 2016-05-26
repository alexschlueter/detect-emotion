#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include "controls.h"
#include "opencv2/opencv.hpp"
#include <functional>
#include <map>
using namespace std;

void onTrackbarChange(int pos, void *userdata) {
    *(int*)userdata = pos;
}

void skipFrames(int &currentFrame, int toSkip)
{
    cout << "Skipping " << toSkip << " frames." << endl;
    currentFrame += toSkip;
}

void toggleLandmarkNumbers(bool &showLandmarkNumbers)
{
    string onOrOff = showLandmarkNumbers ? "off" : "on";
    cout << "Toggling landmark numbers " << onOrOff << "." << endl;
    showLandmarkNumbers = !showLandmarkNumbers;
}

void togglePause(bool &paused)
{
    string resumeOrPause = paused ? "Resuming." : "Pausing.";
    cout << resumeOrPause << endl;
    paused = !paused;
}

void selectPerson(int &selectedPerson, int select)
{
    cout << "Selecting person " << select << "." << endl;
    selectedPerson = select;
}

int main() {
    array<string, 12> aulabels = {"Inner Brow Raiser", "Outer Brow Raiser", "Brow Lowerer",
                                  "Upper Lid Raiser", "Cheek Raiser", "Nose Wrinkler",
                                  "Lip Corner Puller", "Lip Corner Depressor", "Chin Raiser",
                                  "Lip Stretcher", "Lips Part", "Jaw Drop"};
    int font = cv::FONT_HERSHEY_SIMPLEX;
    string lmpath = "../Landmark_Points_bin/landmarks0";
    string aupath = "../ActionUnit_Labels/SN0";

    typedef vector<array<cv::Point2f, 66>> VideoT;
    typedef vector<array<int, 12>> ActionUnitsT;

    array<VideoT, 10> videos;
    array<ActionUnitsT, 10> aus;

    int maxFrames = 0;
    for (int k = 0; k < 10; ++k) {

        cout << k << endl;

        string filestr;
        if (k < 9) {
            filestr += "0";
        }
        filestr += to_string(k + 1);

        ifstream lmfile(lmpath + filestr + ".bin", ios::binary);
        ifstream aufile(aupath + filestr + "/SN0" + filestr + "_AUs.txt");

        string auline;
        getline(aufile, auline);

        int j;
        for (j = 0; getline(aufile, auline); ++j) {
            array<int, 12> frameaus;
            for (int au = 0; au < 12; ++au) {
                frameaus[au] = stoi(auline.substr(5 + 2 * au, 1));
            }
            aus[k].push_back(frameaus);
            videos[k].push_back({});
            for (int i = 0; i < 66; ++i) {
                lmfile.read((char*)&videos[k][j][i].x, sizeof(float));
                lmfile.read((char*)&videos[k][j][i].y, sizeof(float));
            }
        }
        if (j > maxFrames) maxFrames = j;
    }

    cout << "loaded!" << endl;

    int f = 0;
    int selected = -1;
    bool lmNums = false;
    bool paused = false;

    /** Binding controls. */
    auto controls = Controls({
        {Controls::KEY_SPACE, std::bind(togglePause, std::ref(paused))},
        // {Controls::KEY_LEFT, std::bind(skipFrames, std::ref(f), -1)},
        // {Controls::KEY_RIGHT, std::bind(skipFrames, std::ref(f), +1)},
        {Controls::KEY_LEFT, []{}},
        {Controls::KEY_RIGHT, []{}},
        {Controls::KEY_N, std::bind(toggleLandmarkNumbers, std::ref(lmNums))}
    });
    for(int key = Controls::KEY_0; key <= Controls::KEY_9; key++)
    {
        controls.Bind(key, std::bind(selectPerson, std::ref(selected), key - Controls::KEY_0));
    }

    double fontHeight = cv::getTextSize("0", font, 1, 1, nullptr).height;

    cv::namedWindow("Image", CV_WINDOW_NORMAL);
    cv::createTrackbar("Frame", "Image", nullptr, maxFrames - 1, onTrackbarChange, &f);
    const int vidH = 768;
    const int vidW = 1024;
    cv::Mat all = cv::Mat(2 * vidH, 5 * vidW, CV_8UC1);
    for (;;) {
        cv::setTrackbarPos("Frame", "Image", f);
        if (selected == -1) {
            all = cv::Scalar(255);
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 5; ++j) {
                    const int idx = i * 5 + j;
                    if (f >= videos[idx].size()) break;

                    cv::Mat thisimg = cv::Mat(all, {i * vidH, (i + 1) * vidH}, {j * vidW, (j + 1) * vidW});

                    cv::putText(thisimg, to_string(idx), {100, 150}, font, 5, {0, 0, 0});
                    for (int k = 0; k < 66; ++k) {
                        cv::Point2f center = videos[idx][f][k];
                        cv::circle(thisimg, center, 3, {0, 0, 0});
                    }
                }
            }
            cv::imshow("Image", all);
        } else if (f < videos[selected].size()) { // selected != -1
            cv::Mat thisimg = cv::Mat(vidH, vidW, CV_8UC1, cv::Scalar(255));;

            cv::putText(thisimg, "Subject " + to_string(selected), {50, 50}, font, 1, {0, 0, 0});
            for (int au = 0; au < 12; ++au) {
                cv::putText(thisimg, to_string(aus[selected][f][au]) + " " + aulabels[au], cv::Point2f(50, 70 + (au + 1) * fontHeight * 1.2), font, 1, {0, 0, 0});
            }
            for (int k = 0; k < 66; ++k) {
                cv::Point2f center = videos[selected][f][k];
                cv::circle(thisimg, center, 3, {0, 0, 0});
                if (lmNums) {
                    cv::putText(thisimg, to_string(k), cv::Point2f(center.x + 2, center.y - 2), font, 0.3, {0, 0, 0});
                }
            }
            cv::imshow("Image", thisimg);
        }
        int k = cv::waitKey(50);
        if(!controls.HandleInput(k) && k != -1)
            selected = -1;

        if (!paused)
            ++f;
    }
}