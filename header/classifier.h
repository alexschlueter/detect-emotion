#ifndef CLASSIFIACTOR_H
#define CLASSIFIACTOR_H

#include <vector>
#include <opencv2/core/core.hpp>
#include <ostream>
#include <memory>
#include <array>
#include <istream>

enum ClassifierResult{
    noaction = 0, action = 1, failure = -1
};

/**
 *  Subclasses should be able to classify
 *  if a features belongs into a specific
 *  action unit.
 *  Classifier can be trained/created by
 *  ClassifierConstructor
 */
class Classifier{
  public:
    /**
     * Classify every feature using the classify-function.
     */
    inline cv::Mat classifyList(const cv::Mat & features) const {
        cv::Mat result(features.rows, 1, CV_8S);
        for(int r = 0; r < features.rows; r++)
        {
            result.at<int8_t>(r, 0) = this->classify(features.row(r));
        }
        return result;
    }
    /**
     * Serialize the Classifier into a file.
     * (because cv::FileStorage does not support streams)
     * It can be deserialized by using ClassifierConstructor
     * @param stream the istream to save the Classifier to.
     * @param true if success, false otherwise
     */
    virtual bool serialize(const std::string & filename) const = 0;

    /**
     * Classify a feature into a specific class.
     * E.g. if an action happened => 1, else 0
     * On failure -1 should be returned. (E.g. not enough features)
     * @param feature a cv::Mat which represents the features
     * @return the class, -1 on failure
     */
    virtual ClassifierResult classify(const cv::Mat & feature) const = 0;
};



/**
 * Subclasses should be able to create a specific Classifier.
 */
class ClassifierConstructor{
public:
  /**
   * Trains a Classifier using the trainingsset and truthset.
   * The rows in trainingsset describes a trainingssample,
   * the rows in truthsset the results.
   * Therefore truthset should have 1 column.
   * Trainingsset should be a 1-dim float  cv::Mat;
   * Result can be null on failure
   */
  virtual std::unique_ptr<Classifier> train(const cv::Mat & trainingsset, const cv::Mat & truthsset) const= 0;
  /**
   * Deserialize a classifier from a file
   * (because cv::FileStorage does not support streams)
   * Result can be null on failure
   */
  virtual std::unique_ptr<Classifier> deserialize(const std::string & filename) const = 0;
};

using ConfusionMatrix=std::array<std::array<int,2>,2>;
ConfusionMatrix computeConfusionMatrixFromTestSet(const Classifier & c, const cv::Mat& testset, const cv::Mat & truth);

inline double computeRecall(const ConfusionMatrix & cm){
    // TP / (TP + FP)
    return static_cast<double>(cm[0][0])/static_cast<double>(cm[0][0]+cm[1][0]);
}

inline double computePrecision(const ConfusionMatrix & cm){
    // TP / (TP + FN)
    return static_cast<double>(cm[0][0])/static_cast<double>(cm[0][0]+cm[0][1]);
}


#include <opencv2/ml/ml.hpp>
class SVMConstructor: public ClassifierConstructor{
public:
   /**
   * Initialize using params for training SVM
   */
    explicit SVMConstructor(CvSVMParams params);
  /**
   * Initialize using default parameters for training SVM
   **/
  SVMConstructor();
  virtual std::unique_ptr<Classifier> train(const cv::Mat & trainingsset, const cv::Mat& truthset) const;
  virtual std::unique_ptr<Classifier> deserialize(const std::string & filename) const;
private:
    CvSVMParams _params;
};

class KNearestNeighborsConstructor: public ClassifierConstructor{
public:
   /**
   * Initialize using k neighbors
   */
  explicit KNearestNeighborsConstructor(int k = 1);
  virtual std::unique_ptr<Classifier> train(const cv::Mat & trainingsset, const cv::Mat& truthset) const;
  virtual std::unique_ptr<Classifier> deserialize(const std::string & filename) const;
private:
    int _k;
};
#endif /* ifndef CLASSIFIACTOR_H */
