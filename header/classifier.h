#ifndef CLASSIFIACTOR_H
#define CLASSIFIACTOR_H 

#include <vector>
#include <opencv2/core.hpp>
#include <ostream>
#include <memory>
#include <array>
#include <istream>

using FeatureList = std::vector<cv::Mat>;
using TruthList = std::vector<int>;

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
    inline TruthList classifyList(const FeatureList & features) const {
      TruthList result;
      for (const cv::Mat & f : features){
        result.push_back(this->classify(f));
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
    virtual int classify(const cv::Mat & feature) const = 0;
};



/**
 * Subclasses should be able to create a specific Classifier.
 */
class ClassifierConstructor{
public:
  /**
   * Trains a Classifier using the trainingsset and truthset.
   * Result can be null on failure
   */
  virtual std::unique_ptr<Classifier> train(const FeatureList & trainingsset, const TruthList & truthsset) const= 0;
  /**
   * Deserialize a classifier from a file
   * (because cv::FileStorage does not support streams)
   * Result can be null on failure
   */
  virtual std::unique_ptr<Classifier> deserialize(const std::string & filename) const = 0;
};

using ConfusionMatrix=std::array<std::array<int,2>,2>;
ConfusionMatrix computeConfusionMatrixFromTestSet(const Classifier & c, const FeatureList& testset, const TruthList & truth);

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
  virtual std::unique_ptr<Classifier> train(const FeatureList & trainingsset, const TruthList& truthset) const;
  virtual std::unique_ptr<Classifier> deserialize(const std::string & filename) const;
private:
    CvSVMParams _params;
};

#endif /* ifndef CLASSIFIACTOR_H */
