#pragma once
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>

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
        _variance = calculateRetainedVariance(data, dataAsRow);
    }

    /**
     * Invokes a principal component analysis on data, retaining at least the given variance.
     * As default, it expects samples in rows and features in cols.
     */
    void analyseOnVariance(const cv::Mat &data, double retainedVariance = 0.9, bool dataAsRow = true)
    {
        _pca = cv::PCA(data, cv::Mat(), dataAsRow ? CV_PCA_DATA_AS_ROW : CV_PCA_DATA_AS_COL, retainedVariance);
        _variance = calculateRetainedVariance(data, dataAsRow);
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

    float calculateVectorVariance(const cv::Mat &data, float mean)
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

    float calculateRetainedVariance(const cv::Mat &data, bool dataAsRow = true)
    {
        // calculate the total variance of the dataset
        float totalVariance = 0;
        int numIterations = dataAsRow ? data.cols : data.rows;
        for(int i = 0; i < numIterations; i++)
        {
            totalVariance += calculateVectorVariance(dataAsRow ? data.col(i) : data.row(i),
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
