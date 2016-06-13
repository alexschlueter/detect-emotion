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
#include "PointCloud.h"
using namespace std;

const float RESOLUTION = 2048;
const int FONT = cv::FONT_HERSHEY_SIMPLEX;
const bool PRINT_INDICES = false;

void printMatrix(const cv::Mat &mat)
{
    std::cout << std::setprecision(3) << std::endl;
    for(int i=0; i< mat.rows; i++)
    {
        for(int j=0; j<mat.cols; j++)
        {
            std::cout << mat.at<float>(i,j) << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void drawAxis(cv::Mat& img, cv::Point2f p, cv::Point2f q, cv::Scalar colour, const float scale = 0.2)
{
    double angle;
    double hypotenuse;
    angle = std::atan2( (double) p.y - q.y, (double) p.x - q.x ); // angle in radians
    hypotenuse = cv::sqrt( (double) (p.y - q.y) * (p.y - q.y) + (p.x - q.x) * (p.x - q.x));
//    double degrees = angle * 180 / CV_PI; // convert radians to degrees (0-180 range)
//    cout << "Degrees: " << abs(degrees - 180) << endl; // angle in 0-360 degrees range
    // Here we lengthen the arrow by a factor of scale
    q.x = (int) (p.x - scale * hypotenuse * std::cos(angle));
    q.y = (int) (p.y - scale * hypotenuse * std::sin(angle));
    cv::line(img, p, q, colour, 1, CV_AA);
    // create the arrow hooks
    p.x = (int) (q.x + 9 * std::cos(angle + CV_PI / 4));
    p.y = (int) (q.y + 9 * std::sin(angle + CV_PI / 4));
    cv::line(img, p, q, colour, 1, CV_AA);
    p.x = (int) (q.x + 9 * std::cos(angle - CV_PI / 4));
    p.y = (int) (q.y + 9 * std::sin(angle - CV_PI / 4));
    cv::line(img, p, q, colour, 1, CV_AA);
}

void pcaAndDraw(cv::Mat &img, cv::Mat &features)
{
    int maxComponents = 5;
    cv::PCA pca_analysis(features, cv::Mat(), CV_PCA_DATA_AS_ROW, maxComponents);

    printMatrix(pca_analysis.eigenvectors);


    for(int i = 0; i < features.rows; i++)
    {
        cv::circle(img, cv::Point2f(features.at<float>(i, 0), std::fabs(features.at<float>(i, 1)) * 300), 1, cv::Scalar(255,0,0));
    }

    cv::Mat compressed;
    compressed.create(features.rows, maxComponents, features.type());
    auto outmat = cv::OutputArray(compressed).getMat();
    printMatrix(outmat);

    cv::Mat reconstructed;
    for( int i = 0; i < features.rows; i++ )
    {
        cv::Mat vec = features.row(i);
        cv::Mat coeffs = outmat.row(i);
        pca_analysis.project(vec, coeffs);
        // and then reconstruct it
        pca_analysis.backProject(coeffs, reconstructed);
        // and measure the error
        printf("%d. diff = %g\n", i, cv::norm(vec, reconstructed, cv::NORM_L2));
    }
}

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
    cv::Mat img;
    FeatureExtractionAggregate<66> featureExtractor;
    featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new EuclideanDistanceExtraction<66>()));
    featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new OrientationExtraction<66>()));
    cv::Mat featureSet = extractFeaturesFromData<66>(pointClouds, &featureExtractor);


    /* Print */
    std::cout << landmarks.size() << " frames" << std::endl;
    std::cout << "Feature set is " << featureSet.rows << "x" << featureSet.cols << std::endl;
    std::cout << "Starting PCA..." << std::endl;
    cv::PCA pca(featureSet, cv::Mat(), CV_PCA_DATA_AS_ROW, 6);
    std::cout << "Finished PCA..." << std::endl;
    cv::Mat pointMat(1, 2*66, CV_32FC1);

    cv::namedWindow("Image");
    for (auto frame : pointClouds) {
        cv::Point2f imageSize(frame.greatestXDistance() * 2, frame.greatestYDistance() * 2);
        cv::Point2f quarterImageSize(imageSize.x / 2.f, imageSize.y / 2.f);

        img = cv::Mat::zeros(imageSize.y, imageSize.x, CV_8UC3);

        // get matrix from points
        for(int i = 0; i < 66; i++)
        {
            pointMat.at<float>(0, 2*i) = frame[i].x;
            pointMat.at<float>(0, 2*i + 1) = frame[i].y;
        }
        cv::Mat projection;
        pca.project(featureSet, projection);
        std::cout << "Projection has size " << projection.rows << "x" << projection.cols << std::endl;

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
