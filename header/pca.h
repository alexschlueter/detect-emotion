#ifndef PCA_H
#define PCA_H
#include "pointcloud.h"
#include <opencv2/core/core.hpp>
#include <vector>
#include <opencv/cv.hpp>
#include "normalization.h"

template <unsigned int N>
PointCloud<N> standardNormalization(const PointCloud<N> & points){
    auto tmp = centerPointCloud<N>(points);
    tmp = rotatePointCloudOnEyeAndForehead<N>(tmp);
    return  scalePointCloud2<N>(tmp,1);
}

#include <QDebug>
template <unsigned int N = 66>
struct PCA_Result{
    cv::PCA _pca;
    //std::array<unsigned int,2*N> permutation;

    PCA_Result(cv::PCA & pca): _pca(pca){
        // Size has to be 2*N (N points with x and y coordinates)
        assert(_pca.eigenvectors.cols == 2*N);
        assert(_pca.eigenvectors.rows == 2*N);
        assert(_pca.eigenvalues.rows == 2*N);
        assert(_pca.eigenvalues.cols == 1);
        assert(_pca.mean.cols == 2*N);
        assert(_pca.mean.rows == 1);
        assert(_pca.mean.type() == CV_32FC1);

        /*for (int i=0; i< 2*N; i++) permutation[i] = i;
       std::sort(permutation.begin(),permutation.end(),[this](int a, int b){
          return _pca.eigenvalues.at<float>(0, a) < _pca.eigenvalues.at<float>(0,b);
       });*/
    }

    PCA_Result(PCA_Result && other){
        _pca = std::move(other._pca);
    }

    template <typename F = decltype(standardNormalization<N>)>
    PointCloud<N> rebuild(const PointCloud<N> & _cloud, int dimensions, F normFunc = standardNormalization) const{
        if (dimensions > _pca.eigenvectors.cols){ // Invalid Input TODO: Error message
            dimensions = _pca.eigenvectors.cols;
        }
        auto normCloud = normFunc(_cloud);
        PointArray<N> res;
        cv::Mat_<float> input(1,2*N);
        float * inputRow = input[0];
        for (int i=0; i < N; i++){
            inputRow[2*i] = normCloud[i].x;//- _pca.mean.at<float>(0,2*i);
            inputRow[2*i+1] = normCloud[i].y;//- _pca.mean.at<float>(0,2*i+1);
        }
        cv::Mat projection = _pca.project(input);
        assert(projection.type()==CV_32FC1);
        float* projRow = projection.ptr<float>(0);
        for(int i=2*dimensions; i<2*N; i++){
            //projRow[permutation[i]] = 0;
            projRow[i] = 0;
        }
        cv::Mat_<float> result(1,2*N);
        float * resultRow = result[0];
        _pca.backProject(projection,result);
        for (int i=0; i< N; i++){
            res[i].x = resultRow[2*i] ;//+ _pca.mean.at<float>(0,2*i);
            res[i].y = resultRow[2*i+1];//+ _pca.mean.at<float>(0,2*i+1);
        }
        return PointCloud<N>(std::move(res));
    }
};


template <unsigned int N, typename F = decltype(standardNormalization<N>)>
PCA_Result<N> computePCA(const std::vector<PointCloud<N>>& _pointcloud, F normFunc = standardNormalization<N>){
    //Normalize PointCloudstatic_cast<float>(
    std::vector<PointCloud<N>> pointcloud(_pointcloud);
    applyOnNormalizationOnPointCloud(pointcloud, normFunc);

    //
    cv::Mat_<float> pca_mat(pointcloud.size(),2*N);
    for (int y = 0; y < pointcloud.size(); y++){
        for (int x = 0; x < N; x++){
            pca_mat.at<float>(y,2*x) = pointcloud[y][x].x;
            pca_mat.at<float>(y,2*x+1) = pointcloud[y][x].y;
        }
    }
    cv::PCA pca(pca_mat,cv::Mat(),CV_PCA_DATA_AS_ROW);
    assert(pca.eigenvectors.type() == CV_32FC1);
    return PCA_Result<N>(pca);
}


#endif
