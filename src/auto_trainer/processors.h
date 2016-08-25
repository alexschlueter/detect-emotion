#ifndef PROCESSORS_H
#define PROCESSORS_H
#include <opencv2/core.hpp>
#include "wrapper.h"

class CloudProcessor{
public:
    virtual ~CloudProcessor(){}
    virtual void analyse(const CloudAction & ) = 0;
    virtual CloudAction apply(const CloudAction &) const = 0;
    virtual std::string name() const= 0;
    virtual void save(const std::string & filename) const = 0;
    virtual bool onlyOnTrainingSet() const{ return false;}
    virtual bool serializable() const = 0;
};

using MatList = std::vector<cv::Mat>;
class FeatureProcessor{
public:
    virtual ~FeatureProcessor(){}
    virtual void analyse(const FeatureTruth&) = 0;
    virtual FeatureTruth apply(const FeatureTruth&) const = 0;
    virtual std::string name() const = 0;
    virtual void save(const std::string& filename ) const = 0;
    virtual bool onlyOnTrainingSet() const{ return false;}
    virtual bool serializable() const = 0;
};

class RandomJitterExpander:public CloudProcessor{
public:
    RandomJitterExpander(double meanx, double stdx, double meany, double stdy);
    virtual void analyse(const CloudAction & );
    virtual CloudAction apply(const CloudAction &) const;
    virtual std::string name() const;
    virtual void save(const std::string & filename) const;
    virtual bool serializable() const{ return false; }
private:
    double _meanx, _meany, _stdx, _stdy;
};

class CloudNormalizer: public CloudProcessor{
public:
    virtual void analyse(const CloudAction & ) ;
    virtual CloudAction apply(const CloudAction &) const ;
    virtual std::string name() const;
    virtual void save(const std::string & filename) const;
    virtual bool serializable() const{ return false; }
};

class CloudMask: public CloudProcessor{
public:
    CloudMask(std::ifstream & file);
    virtual void analyse(const CloudAction & ) ;
    virtual CloudAction apply(const CloudAction &) const ;
    virtual std::string name() const;
    virtual void save(const std::string & filename) const;
    virtual bool serializable() const{ return false; }
private:
    std::vector<int> _toKeep;
};

#include "pca.h"
class PCACloudReducer: public CloudProcessor{
public:
    PCACloudReducer(unsigned int dimension);
    virtual void analyse(const CloudAction & ) ;
    virtual CloudAction apply(const CloudAction &) const ;
    virtual std::string name() const;
    virtual void save(const std::string & filename) const;
    static  std::unique_ptr<PCACloudReducer> load(const std::string &filename);
    virtual bool serializable() const{ return true; }
private:
    unsigned int _dimension;
    std::unique_ptr<PCA_Result<66>> _pca;
};

#include "pcanalysis.h"
class PCAFeatureReducer: public FeatureProcessor{
public:
    PCAFeatureReducer(double retain_variance);
    virtual void analyse(const FeatureTruth&);
    virtual FeatureTruth apply(const FeatureTruth&) const;
    virtual std::string name() const ;
    virtual void save(const std::string& filename ) const;
    static std::unique_ptr<PCAFeatureReducer> load(const std::string &filename);
    virtual bool serializable() const{ return true; }
private:
    double _retain_variance;
    PCAnalysis _pca;
};

class FeatureShuffler: public FeatureProcessor{
public:
    virtual void analyse(const FeatureTruth&);
    virtual FeatureTruth apply(const FeatureTruth&) const;
    virtual std::string name() const ;
    virtual void save(const std::string& filename ) const;
    virtual bool serializable() const{ return false; }
};

class PersonShuffler: public CloudProcessor{
public:
    virtual void analyse(const CloudAction & ) ;
    virtual CloudAction apply(const CloudAction &) const ;
    virtual std::string name() const;
    virtual void save(const std::string & ) const;
    virtual bool serializable() const{ return false; }
};

class ReduceNegatives: public FeatureProcessor{
public:
    ReduceNegatives(double negativesToPositives);
    virtual void analyse(const FeatureTruth&);
    virtual FeatureTruth apply(const FeatureTruth&) const;
    virtual std::string name() const ;
    virtual void save(const std::string& filename ) const;
    virtual bool onlyOnTrainingSet() const{ return true;}
    virtual bool serializable() const{ return false; }
private:
    double _negativesToPostives;
};

#include "featurescaling.h"
class FeatureMinMaxNormalizer: public FeatureProcessor{
public:
    virtual void analyse(const FeatureTruth&);
    virtual FeatureTruth apply(const FeatureTruth&) const;
    virtual std::string name() const ;
    virtual void save(const std::string& filename ) const;
    static std::unique_ptr<FeatureMinMaxNormalizer> load(const std::string &filename);
    virtual bool serializable() const{ return true; }
private:
    FeatureScaling _scaler;
};

#endif // PROCESSORS_H
