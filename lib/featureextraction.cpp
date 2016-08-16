#include "featureextraction.h"
using namespace std;

cv::Mat_<float> polynomInterpolation(vector<cv::Point2f> points, unsigned int degree){
    double meanx = 0;
    double meany = 0;
    cv::Point2f startx=points[0], endx=points[0];
    for (auto p: points){
        meanx += p.x;
        meany += p.y;
        if (startx.x > p.x) startx = p;
        if (endx.x < p.x) endx = p;
    }
    meanx /= points.size();
    meany /= points.size();
    float scale = endx.x - startx.x;
    cv::Mat solution(points.size(),1,CV_32FC1);
    cv::Mat M(points.size(),degree+1,CV_32FC1);
    for(int i=0; i<points.size();i++){
        float* row = M.ptr<float>(i);
        float x = (points[i].x - meanx)/scale;
        float xpow = 1;
        row[0] = xpow;
        for(int j=1; j < degree+1; j++){
            xpow = xpow * x;
            row[j] = xpow;
        }
        solution.at<float>(i,0) = (points[i].y - meany)/scale;
    }
    cv::Mat_<float> res(degree+1,1);
    cv::solve(M,solution, res, cv::DECOMP_QR);
    return res;
}

struct PolynomInfo{
    unsigned int degree;
    vector<size_t> idxOfRelevantPoints;
    inline unsigned int numberOfCoefficients(){ return degree+1;}
};

static vector<PolynomInfo> interpolationInfos = {
    PolynomInfo{2, vector<size_t>{36,37,38,39}}, // Right upper eye
    PolynomInfo{2, vector<size_t>{36,41,40,39}}, // Right lower eye
    PolynomInfo{2, vector<size_t>{42,43,44,45}}, // Left upper eye
    PolynomInfo{2, vector<size_t>{42,47,46,45}}, // Left lower eye
    PolynomInfo{4, vector<size_t>{48,59,58,57,56,55,54}}, // Bottom outer lip
    PolynomInfo{4, vector<size_t>{48,49,50,51,52,53,54}}, // Top outer lip
    PolynomInfo{2, vector<size_t>{60,61,62}}, // top inner lip
    PolynomInfo{2, vector<size_t>{65,64,63}}, // bottom inner lip
    PolynomInfo{2, vector<size_t>{31,32,33,34,35}}, // nose bottom
    PolynomInfo{2, vector<size_t>{17,18,19,20,21}}, // right eyebrow
    PolynomInfo{2, vector<size_t>{22,23,24,25,26}} // left eyebrow
};

/*
#include <iostream>
void test_polynom_interp(){
    std::vector<cv::Point2f> points;
    points.push_back(cv::Point2f(-1,-0.6));
    points.push_back(cv::Point2f(0,-2));
    points.push_back(cv::Point2f(2,-0.8));
    points.push_back(cv::Point2f(5,2));
    auto res = polynomInterpolation(points,2);
    assert(abs(res(0,0)+0.1657197) < 0.001);
    assert(abs(res(0,1)-0.31212121) < 0.001);
    assert(abs(res(0,2)-  1.13636364) < 0.001);
}
*/

cv::Mat InterpolationFeatureExtraction::extractFeatures(const PointCloud<66> &pointCloud) const{
    cv::Mat res(1,getNumFeatures(),CV_32FC1);
    float* rowPtr = res.ptr<float>(0);
    for (auto info: interpolationInfos){
        std::vector<cv::Point2f> relevantPoints;
        for(auto idx: info.idxOfRelevantPoints){
            relevantPoints.push_back(pointCloud[idx]);
        }
        auto coef = polynomInterpolation(relevantPoints,info.degree);
        for (int i=0; i < info.numberOfCoefficients(); i++){
            *rowPtr = coef(i,0);
            rowPtr++;
        }
    }
    return res;
}

unsigned int InterpolationFeatureExtraction::getNumFeatures() const{
    unsigned int res=0;
    for(auto el: interpolationInfos){
        res += el.numberOfCoefficients();
    }
    return res;
}
