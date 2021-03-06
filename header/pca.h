#ifndef PCA_H
#define PCA_H
#include "pointcloud.h"
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <vector>
#include <opencv/cv.hpp>
#include "normalization.h"

template <unsigned int N>
PointCloud<N> standardNormalization(const PointCloud<N> & points){
    auto tmp = centerPointCloud<N>(points);
    tmp = rotatePointCloudOnEyeAndForehead<N>(tmp);
    return  scalePointCloud2<N>(tmp,1);
}

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

    template <typename F=decltype(standardNormalization<N>)>
    cv::Mat project(const PointCloud<N> & _cloud,int dimensions,  F normFunc = standardNormalization){
        if (dimensions > _pca.eigenvectors.cols){ // Invalid Input TODO: Error message
            dimensions = _pca.eigenvectors.cols;
        }
        auto normCloud = normFunc(_cloud);
        cv::Mat_<float> input = normCloud.asMat();
        cv::Mat projection = _pca.project(input);
        assert(projection.type()==CV_32FC1);
        cv::Mat result(1,dimensions,CV_32FC1);
        float* resultRow = result.ptr<float>(0);
        float* projRow = projection.ptr<float>(0);
        for(int i=0; i <dimensions; i++){
            resultRow[i] = projRow[i];
        }
        return result;
    }
    PointCloud<N> rebuild(const PointCloud<N> & cloud, int dimensions) const{
        if (2*dimensions > _pca.eigenvectors.cols){ // Invalid Input TODO: Error message
            dimensions = 2*_pca.eigenvectors.cols;
        }
        PointArray<N> res;
        cv::Mat_<float> input = cloud.asMat();
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

    template <typename F = decltype(standardNormalization<N>)>
    PointCloud<N> rebuildWithNorm(const PointCloud<N> & _cloud, int dimensions, F normFunc = standardNormalization) const{
        if (2*dimensions > _pca.eigenvectors.cols){ // Invalid Input TODO: Error message
            dimensions = 2*_pca.eigenvectors.cols;
        }
        auto normCloud = normFunc(_cloud);
        return rebuild(normCloud, dimensions);
    }

    void save(cv::FileStorage & storage){
        storage << "mean" <<_pca.mean;
        storage << "e_vects" <<_pca.eigenvectors;
        storage << "e_vals" << _pca.eigenvalues;
    }

    void save(const std::string & filename){
        cv::FileStorage storage(filename,cv::FileStorage::WRITE);
        save(storage);
        storage.release();
    }

    static PCA_Result<N> load(const std::string & filename){
        cv::FileStorage storage(filename,cv::FileStorage::READ);
        return PCA_Result<N>::load(storage);
    }
    static PCA_Result<N> load(cv::FileStorage & storage){
        cv::PCA pca;
        storage["mean"]>> pca.mean;
        storage["e_vects"]>> pca.eigenvectors;
        storage["e_vals"]>> pca.eigenvalues;
        storage.release();
        return PCA_Result(pca);
    }
};

template <unsigned int N>
PCA_Result<N> computePCA(const std::vector<PointCloud<N>>& _pointcloud){
    //Normalize PointCloudstatic_cast<float>(
    std::vector<PointCloud<N>> pointcloud(_pointcloud);
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


template <unsigned int N, typename F = decltype(standardNormalization<N>)>
PCA_Result<N> computePCAWithNorm(const std::vector<PointCloud<N>>& _pointcloud, F normFunc = standardNormalization<N>){
    //Normalize PointCloudstatic_cast<float>(
    std::vector<PointCloud<N>> pointcloud(_pointcloud);
    applyOnNormalizationOnPointCloud(pointcloud, normFunc);

    return computePCA(pointcloud);
}



#endif
