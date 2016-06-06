#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include "controls.h"
#include "reader.h"
#include "actionunit.h"
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

cv::Mat normalize(const cv::Mat& src) {
    cv::Mat srcnorm;
    cv::normalize(src, srcnorm, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    return srcnorm;
}

cv::PCA compressPCA(cv::InputArray pcaset, int maxComponents,
                    const cv::Mat& testset, cv::OutputArray compressed)
{
    cv::PCA pca(pcaset, // pass the data
                cv::Mat(), // there is no pre-computed mean vector,
                   // so let the PCA engine to compute it
            CV_PCA_DATA_AS_ROW, // indicate that the vectors
                                // are stored as matrix rows
                                // (use CV_PCA_DATA_AS_COL if the vectors are
                                // the matrix columns)
            maxComponents // specify how many principal components to retain
            );
    // if there is no test data, just return the computed basis, ready-to-use
    cout << "ddfl"<<endl;
    if( !testset.data )
        return pca;
    cout << "test"<<endl;
    auto outmat = compressed.getMat();
    // CV_Assert( testset.cols == pcaset.cols );

    compressed.create(testset.rows, maxComponents, testset.type());

    cv::Mat reconstructed;
    for( int i = 0; i < testset.rows; i++ )
    {
        cv::Mat vec = testset.row(i), coeffs = outmat.row(i);
        // compress the vector, the result will be stored
        // in the i-th row of the output matrix
        pca.project(vec, coeffs);
        // and then reconstruct it
        pca.backProject(coeffs, reconstructed);
        // and measure the error
        printf("%d. diff = %g\n", i, norm(vec, reconstructed, cv::NORM_L2));
    }
    return pca;
}

int main() {
    int font = cv::FONT_HERSHEY_SIMPLEX;
    string lmpath = "../../Landmark_Points_bin/landmarks0";
    string aupath = "../../ActionUnit_Labels/SN0";

    typedef vector<array<cv::Point2f, 66>> VideoT;

    vector<VideoT> videos;
    vector<ActionUnit> aus;

    cv::Mat featureMat;

    int maxFrames = 0;
    for (int k = 0; k < 10; ++k) {

        cout << k << endl;

        string filestr;
        if (k < 9) {
            filestr += "0";
        }
        filestr += to_string(k + 1);


        string binaryfilename(lmpath + filestr + ".bin");
        videos.push_back(readBinaryFile(binaryfilename));
        for (auto &frame : videos[k]) {
            cv::Mat newrow;
            for (auto &point : frame) {
                newrow.push_back(point.x);
                newrow.push_back(point.y);
            }
            featureMat.push_back(cv::Mat(newrow.t()));
        }

        string actionfilename(aupath + filestr + "/SN0" + filestr + "_AUs.txt");
        auto actionunit = readActionUnitFromFile(actionfilename);
        aus.push_back(actionunit);

        maxFrames = std::max(maxFrames,actionunit.mat.rows-1);
    }

    cout << "loaded!" << endl;

    cv::Mat(compressed);
    auto pca = compressPCA(featureMat, CV_PCA_DATA_AS_ROW, 10, featureMat, compressed);
    cout << "derp";

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
            auto actionunit = aus.at(selected);
            auto actionlabels = actionunit.getActionsAsName();
            auto actionintens = actionunit.getActionsIntensity(f + 1);
            for (int i=0; i< actionlabels.size();i++){
                cv::putText(thisimg, to_string(actionintens[i]) + " " + actionlabels[i], cv::Point2f(50, 70 + (i + 1) * fontHeight * 1.2), font, 1, {0, 0, 0});
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
        if (k=='q') break;

        if (!paused)
            ++f;
    }
}
