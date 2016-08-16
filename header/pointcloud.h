#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <opencv2/core/core.hpp>
#include "math.h"
#include <limits>

template <typename T>
class ImplicitSharedPtr{

private:

    struct PtrPriv{
            T _val;
            int _refcounter;
    };


public:
    ImplicitSharedPtr():_d(nullptr){}
    ImplicitSharedPtr(const T& val){
        _d = new PtrPriv{val, 1};
    }

    ImplicitSharedPtr(T&& val){
        _d = new PtrPriv{std::move(val), 1};
    }

    ImplicitSharedPtr(const ImplicitSharedPtr<T> & other){
        _d = other._d;
        _d->_refcounter++;
    }

    ImplicitSharedPtr(ImplicitSharedPtr<T> &&other){
        _d = other._d;
        other._d = nullptr;
    }

    ~ImplicitSharedPtr(){
       deref();
    }

    T& operator*(){
        detach();
        return _d->_val;
    }

    const T& operator*() const{
        return _d->_val;
    }

    T* operator->(){
        detach();
        return &_d->val;
    }

    void operator=(const ImplicitSharedPtr<T> & other){
       deref();
       _d = other._d;
       _d->_refcounter++;
    }

    void operator=(ImplicitSharedPtr<T> && other){
        deref();
        _d = other._d;
        other._d = nullptr;
    }


    const T* operator->() const{
        return &_d->_val;
    }
private:
    void detach(){
        if (_d->_refcounter==1) return;
        PtrPriv* other = _d;
        _d = new PtrPriv{_d->_val /*copy*/, 1};
        other->_refcounter--;
    }

    inline void deref(){
        if (_d != nullptr){
            if (_d->_refcounter == 1) {delete _d; _d=nullptr;}
            else {_d->_refcounter--;}
        }
    }

    mutable PtrPriv* _d;
};

template <unsigned int N>
using PointArray =  std::array<cv::Point2f, N> ;

inline float getRadiansBetweenPoints(const cv::Point2f &p1, const cv::Point2f &p2)
{
    return std::atan2(p1.y, p1.x) - std::atan2(p2.y, p2.x);
}

inline cv::Point2f rotatePoint(const cv::Point2f &point, float radians)
{
    cv::Point2f result;
    result.x = point.x * std::cos(radians) - point.y * std::sin(radians);
    result.y = point.x * std::sin(radians) + point.y * std::cos(radians);
    return result;
}

/**
* Add every point from other to PointCloud's points
* elementwise with factor
* => result = a + factor*b
* @param a some Points
* @param b Points to add with some factor
* @param factor value to multiply on every addition
* @return  PointCloud with added points
*/
template <unsigned int N>
PointArray<N> addPointArray(const PointArray<N> & a, const PointArray<N> & b, float factor=1){
    PointArray<N> res;
    for (int i=0; i< N; i++){
        res[i] = a[i] + b[i]*factor;
    }
    return res;
}

/**
 * Useful operation on point cloud
 */
template <unsigned int N=66>
class PointCloud{
public:
    PointCloud() {}
    PointCloud(const PointArray<N> & points): _points(points){}
    PointCloud(PointArray<N> && points): _points(std::move(points)){}
    PointCloud(const PointCloud &other): _points(other._points){}
    PointCloud(PointCloud && other): _points(std::move(other._points)){}

    void operator=(const PointCloud & other){
        this->_points = other._points;
    }

    void operator=(PointCloud && other){
        this->_points = std::move(other._points);
    }

    cv::Point2f midPoint() const{
        cv::Point2f center(0, 0);
        for(auto p : *_points) {
            center += p;
        }
        return center *(1.0/ float(_points->size()));
    }

    /**
     * Translate the points such that the point newZero becomes
     * (0,0) in the cloud.
     * @param newZero new (0,0)
     * @return new point cloud
     */
    PointCloud translate(const cv::Point2f & newZero) const{
        PointArray<N> newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i] = (*_points)[i]-newZero;
        }
        return PointCloud(newPoints);
    }

    PointCloud scale(float factor) const{
        PointArray<N> newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i] = (*_points)[i]*factor;
        }
        return PointCloud(newPoints);
    }

    PointCloud scaleX(float factor) const{
        PointArray<N> newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i].x = (*_points)[i].x*factor;
            newPoints[i].y = (*_points)[i].y;
        }
        return PointCloud(newPoints);
    }

    PointCloud scaleY(float factor) const{
        PointArray<N> newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i].x = (*_points)[i].x;
            newPoints[i].y = (*_points)[i].y*factor;
        }
        return PointCloud(newPoints);
    }

    PointCloud rotate(float angle) const{
        PointArray<N> newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i] = rotatePoint((*_points)[i],angle);
        }
        return PointCloud(newPoints);
    }

    /**
     * Compute the greatest distance
     * of the x-coordinates in the cloud
     * @return greatest distance
     */
    float greatestXDistance() const{
        float min = std::numeric_limits<float>::infinity();
        float max = -std::numeric_limits<float>::infinity();
        for (auto & p: *_points){
            min = std::min(min,p.x);
            max = std::max(max,p.x);
        }
        return max - min;
    }

    /**
     * Compute the greatest distance
     * of the y-coordinates in the cloud
     * @return greatest distance
     */
    float greatestYDistance() const{
        float min = std::numeric_limits<float>::infinity();
        float max = -std::numeric_limits<float>::infinity();
        for (auto & p: *_points){
            min = std::min(min,p.y);
            max = std::max(max,p.y);
        }
        return max - min;
    }

    inline const PointArray<N>& points() const { return *_points;}
    inline PointArray<N> & points() {return *_points;}

    inline const cv::Point2f & operator[](std::size_t i) const{
        return (*_points)[i];
    }

    inline cv::Point2f & operator[](std::size_t i) {
        return (*_points)[i];
    }

    /**
     * Add every point from other to PointCloud's points
     * elementwise
     * @param other Points to add
     * @return  PointCloud with added points
     */
    PointCloud<N> operator+(const PointArray<N>& other) const{
        PointArray<N> res;
        for (int i=0; i< N; i++){
            res[i] = (*_points)[i] + other[i];
        }
        return PointCloud<N>(res);
    }

    /**
     * Put every points into a cv::Mat;
     * The mat has dimension 1x2N;
     */
    cv::Mat_<float> asMat() const{
        cv::Mat_<float> res(1,2*N);
        float * row = res[0];
        for (int i=0; i < N; i++){
            row[2*i] = (*_points)[i].x;
            row[2*i+1] = (*_points)[i].y;
        }
        return res;
    }

private:
    ImplicitSharedPtr<PointArray<N>> _points;
};


#endif // POINTCLOUD_H
