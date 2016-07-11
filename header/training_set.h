#pragma once
#include <opencv2/core/core.hpp>
#include <stdexcept>
#include <random>

class TrainingSet
{
public:
    TrainingSet(const cv::Mat& _features, const cv::Mat& _labels, float trainingAmount = 0.5, float testAmount = 0.25, float validationAmount = 0.25)
    : trainingRangeEnd(0), testRangeEnd(0), validationRangeEnd(0)
    {
        if(!reset(_features, _labels))
            throw std::runtime_error("TrainingSet features and labels need to have the same row dimension.");

        setSplit(trainingAmount, testAmount, validationAmount);
    }

    bool reset(const cv::Mat& _features, const cv::Mat& _labels)
    {
        if(_features.rows != _labels.rows)
            return false;
        features = _features.clone();
        labels = _labels.clone();
        labelsIndicated = labels.clone();
        cv::Mat mask = labels > 0;
        labelsIndicated.setTo(1, mask);
        return true;
    }

    void setSplit(float trainingAmount, float testAmount, float validationAmount)
    {
        float sumPercent = trainingAmount + testAmount + validationAmount;
        trainingAmount /= sumPercent;
        testAmount /= sumPercent;
        validationAmount /= sumPercent;

        int n = features.rows;
        trainingRangeEnd = n * trainingAmount;
        testRangeEnd = trainingRangeEnd + n * testAmount;
        validationRangeEnd = testRangeEnd + n * validationAmount;
    }

    std::size_t size()
    {
        return features.rows;
    }

    void shuffle()
    {
        // copy the matrices
        cv::Mat featuresCopy = features.clone(), labelsCopy = labels.clone(), labelsIndicatedCopy = labelsIndicated.clone();

        // create indices
        std::vector<std::size_t> indices(size());
        for(int r = 0; r < indices.size(); r++)
            indices[r] = r;

        // shuffle the indices
        std::random_device rd;
        std::mt19937 generator(rd());
        std::shuffle(indices.begin(),indices.end(),generator);

        // copy shuffled rows
        for(int i=0;i<indices.size();i++)
        {
            int index = indices[i];
            featuresCopy.row(i).copyTo(features.row(index));
            labelsCopy.row(i).copyTo(labels.row(index));
            labelsIndicatedCopy.row(i).copyTo(labelsIndicated.row(index));
        }
    }

    void balance()
    {
        // TODO: implement
    }

    unsigned int trainingSize()
    {
        return trainingRangeEnd;
    }
    unsigned int testSize()
    {
        return testRangeEnd - trainingRangeEnd;
    }
    unsigned int validationSize()
    {
        return validationRangeEnd - testRangeEnd;
    }

    cv::Mat trainingFeatures()
    {
        return features.rowRange(0, trainingRangeEnd).clone();
    }
    cv::Mat testFeatures()
    {
        return features.rowRange(trainingRangeEnd, testRangeEnd).clone();
    }
    cv::Mat validationFeatures()
    {
        return features.rowRange(testRangeEnd, validationRangeEnd).clone();
    }

    // if indicated==true matrix only contains 0 or 1
    cv::Mat trainingLabels(bool indicated = true)
    {
        if(indicated)
            return labelsIndicated.rowRange(0, trainingRangeEnd).clone();
        return labels.rowRange(0, trainingRangeEnd).clone();
    }
    // if indicated==true matrix only contains 0 or 1
    cv::Mat testLabels(bool indicated = true)
    {
        if(indicated)
            return labelsIndicated.rowRange(trainingRangeEnd, testRangeEnd).clone();
        return labels.rowRange(trainingRangeEnd, testRangeEnd).clone();
    }
    // if indicated==true matrix only contains 0 or 1
    cv::Mat validationLabels(bool indicated = true)
    {
        if(indicated)
            return labelsIndicated.rowRange(testRangeEnd, validationRangeEnd).clone();
        return labels.rowRange(testRangeEnd, validationRangeEnd).clone();
    }

private:
    int trainingRangeEnd,
        testRangeEnd,
        validationRangeEnd;

    cv::Mat features,           // FRAMES x FEATURES matrix
            labels,             // FRAMES x LABELS matrix
            labelsIndicated;    // FRAMES x LABELS[0 or 1] matrix
};