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
    std::cout << std::setprecision(15) << std::endl;
    for(int i=0; i< mat.rows; i++)
    {
        for(int j=0; j<mat.cols; j++)
        {
            std::cout << std::fixed << mat.at<float>(i, j) << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

class PCAnalysis
{
public:
    PCAnalysis()
    {
        // empty
    }
    PCAnalysis(const cv::Mat &data, int numComponents = 2, bool dataAsRow = true)
    {
        analyseOnComponents(data, numComponents, dataAsRow);
    }
    PCAnalysis(const cv::Mat &data, double variance = 0.9, bool dataAsRow = true)
    {
        analyseOnVariance(data, variance, dataAsRow);
    }
    PCAnalysis(const std::string &filepath)
    {
        fromFile(filepath);
    }

    /**
     * Invokes a principal component analysis on data, retaining at most the given number of components.
     * As default, it expects samples in rows and features in cols.
     */
    void analyseOnComponents(const cv::Mat &data, unsigned int numComponents = 2, bool dataAsRow = true)
    {
        _pca = cv::PCA(data, cv::Mat(), dataAsRow ? CV_PCA_DATA_AS_ROW : CV_PCA_DATA_AS_COL, (int)numComponents);
        _variance = calculateVariance(data, dataAsRow);
    }

    /**
     * Invokes a principal component analysis on data, retaining at least the given variance.
     * As default, it expects samples in rows and features in cols.
     */
    void analyseOnVariance(const cv::Mat &data, double retainedVariance = 0.9, bool dataAsRow = true)
    {
        _pca = cv::PCA(data, cv::Mat(), dataAsRow ? CV_PCA_DATA_AS_ROW : CV_PCA_DATA_AS_COL, retainedVariance);
        _variance = calculateVariance(data, dataAsRow);
    }

    /** Projects data into the PCA space. */
    void project(const cv::Mat &data, cv::Mat &output) const
    {
        _pca.project(data, output);
    }

    /** Projects data into the PCA space. */
    cv::Mat project(const cv::Mat &data) const
    {
        return _pca.project(data);
    }

    /** Reconstructs original data from their PCA projection. */
    void backProject(const cv::Mat &data, cv::Mat &output) const
    {
        _pca.backProject(data, output);
    }

    /** Reconstructs original data from their PCA projection. */
    cv::Mat backProject(const cv::Mat &data) const
    {
        return _pca.backProject(data);
    }

    /** Returns the number of principal components of the analysis. */
    int getNumComponents() const
    {
        return _pca.eigenvectors.rows;
    }

    /** Returns the dimension of the PCA. */
    int getDimension() const
    {
        return _pca.eigenvectors.cols;
    }

    /** Returns the variance of the PCA. */
    double getVariance() const
    {
        return _variance;
    }

    void print() const
    {
        std::cout << "### PCA ###" << std::endl;
        std::cout << " Mean is " << _pca.mean.rows << "x" << _pca.mean.cols << std::endl;
        std::cout << " Eigenvectors are " << _pca.eigenvectors.rows << "x" << _pca.eigenvectors.cols << std::endl;
        std::cout << " Eigenvalues are " << _pca.eigenvectors.rows << "x" << _pca.eigenvectors.cols << std::endl;
        std::cout << " Number of components are " << getNumComponents() << std::endl;
        std::cout << " Variance is " << std::fixed << _variance << std::endl;
    }

    /** Stores the results of the PCA to a file. */
    void toFile(const std::string &filepath) const
    {
        cv::FileStorage filestorage(filepath, cv::FileStorage::WRITE);
        filestorage << "mean" << _pca.mean;
        filestorage << "eigenvectors" << _pca.eigenvectors;
        filestorage << "eigenvalues" << _pca.eigenvalues;
        filestorage.release();
    }

    /** Tries to load the results of a PCA from a file. */
    bool fromFile(const std::string &filepath)
    {
        cv::FileStorage filestorage(filepath, cv::FileStorage::READ);
        if(!filestorage.isOpened())
            return false;

        try {
            filestorage["mean"] >> _pca.mean;
            filestorage["eigenvectors"] >> _pca.eigenvectors;
            filestorage["eigenvalues"] >> _pca.eigenvalues;
            filestorage.release();
        }
        catch(...)
        {
            filestorage.release();
            return false;
        }
        return true;
    }
private:
    cv::PCA _pca;
    float _variance = -1;

    float calculateSingleEntryVariance(const cv::Mat &data, float mean)
    {
        // make sure this is a vector, not a matrix
        assert(data.rows == 1 || data.cols == 1);

        // sum over all entries with the mean subtracted
        float variance = 0;
        for(int i = 0; i < data.rows; i++)
        {
            for(int j = 0; j < data.cols; j++)
            {
                float delta = data.at<float>(i,j) - mean;
                variance += delta * delta;
            }
        }

        // return the average
        return variance / float(data.rows * data.cols);
    }

    float calculateVariance(const cv::Mat &data, bool dataAsRow = true)
    {
        // calculate the total variance of the dataset
        float totalVariance = 0;
        int numIterations = dataAsRow ? data.cols : data.rows;
        for(int i = 0; i < numIterations; i++)
        {
            totalVariance += calculateSingleEntryVariance(dataAsRow ? data.col(i) : data.row(i),
                                                          dataAsRow ? _pca.mean.at<float>(0, i) : _pca.mean.at<float>(i, 0));
        }

        // calculate the variance the eigenvalues account for
        float eigenVariance = 0;
        for(int i = 0; i < _pca.eigenvalues.rows; i++)
        {
            for(int j = 0; j < _pca.eigenvalues.cols; j++) {
                eigenVariance += _pca.eigenvalues.at<float>(i, j);
            }
        }

        // return the eigenVariance divided by totalVariance
        return (totalVariance == 0) ? -1 : eigenVariance / totalVariance;
    }

};

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
    // featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new EuclideanDistanceExtraction<66>()));
    // featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new OrientationExtraction<66>()));
    featureExtractor.extractions.push_back(std::shared_ptr<FeatureExtractionBase<66>>(new CenterDistanceExtraction<66>()));
    cv::Mat featureSet = extractFeaturesFromData<66>(pointClouds, &featureExtractor);


    /* Print */
    std::cout << landmarks.size() << " frames" << std::endl;
    std::cout << "Feature set is " << featureSet.rows << "x" << featureSet.cols << std::endl;

    std::cout << "Starting PCA..." << std::endl;
    PCAnalysis pca(featureSet, 0.95, true);
    pca.print();


    cv::namedWindow("Image");
    for (auto frame : pointClouds) {
        cv::Point2f imageSize(frame.greatestXDistance() * 2, frame.greatestYDistance() * 2);
        cv::Point2f quarterImageSize(imageSize.x / 2.f, imageSize.y / 2.f);

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
