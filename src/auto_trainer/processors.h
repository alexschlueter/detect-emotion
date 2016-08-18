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
};


using MatList = std::vector<cv::Mat>;
class FeatureProcessor{
public:
    virtual ~FeatureProcessor(){}
    virtual void analyse(const FeatureTruth&) = 0;
    virtual FeatureTruth apply(const FeatureTruth&) const = 0;
    virtual std::string name() const = 0;
    virtual void save(const std::string& filename ) const = 0;
};

class RandomJitterExpander:public CloudProcessor{
public:
    RandomJitterExpander(double meanx, double stdx, double meany, double stdy);
    virtual void analyse(const CloudAction & );
    virtual CloudAction apply(const CloudAction &) const;
    virtual std::string name() const;
    virtual void save(const std::string & filename) const;
private:
    double _meanx, _meany, _stdx, _stdy;
};

class CloudNormalizer: public CloudProcessor{
public:
    virtual void analyse(const CloudAction & ) ;
    virtual CloudAction apply(const CloudAction &) const ;
    virtual std::string name() const;
    virtual void save(const std::string & filename) const;
};

#include "pcanalysis.h"
class PCAFeatureReducer: public FeatureProcessor{
public:
    PCAFeatureReducer(double retain_variance);
    virtual void analyse(const FeatureTruth&);
    virtual FeatureTruth apply(const FeatureTruth&) const;
    virtual std::string name() const ;
    virtual void save(const std::string& filename ) const;

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
};

#endif // PROCESSORS_H
