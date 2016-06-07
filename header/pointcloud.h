#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <opencv2/core/core.hpp>
#include "math.h"
#include <limits>

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
 * Useful operation on point cloud
 */
template <int N=66>
class PointCloud{
public:
    using Points =  std::array<cv::Point2f, N> ;

    PointCloud() {}
    PointCloud(const Points & points): _points(points){}
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
        for(auto p : _points) {
            center += p;
        }
        return center *(1.0/ float(_points.size()));
    }

    /**
     * Translate the points such that the point newZero becomes
     * (0,0) in the cloud.
     * @param newZero new (0,0)
     * @return new point cloud
     */
    PointCloud translate(const cv::Point2f & newZero) const{
        Points newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i] = _points[i]-newZero;
        }
        return PointCloud(newPoints);
    }

    PointCloud scale(float factor) const{
        Points newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i] = _points[i]*factor;
        }
        return PointCloud(newPoints);
    }

    PointCloud rotate(float angle) const{
        Points newPoints;
        for(int i=0; i<N; i++) {
            newPoints[i] = rotatePoint(newPoints[i],angle);
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
        float max = 0;
        for (auto & p: _points){
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
        float max = 0;
        for (auto & p: _points){
            min = std::min(min,p.y);
            max = std::max(max,p.y);
        }
        return max - min;
    }

    inline const Points& points() const { return _points;}

    inline const cv::Point2f & operator[](std::size_t i) const{
            return _points[i];
    }
private:
   Points _points;
};


#endif // POINTCLOUD_H
