#ifndef UTILS_H
#define UTILS_H

#include <array>
#include <type_traits>
#include <opencv2/core/core.hpp>

template <typename T>
T findMaxInArrayHelper(const T & a, const T & b){
	static_assert(std::is_arithmetic<T>::value, "Not Implemented findMaxInArrayHelper");
  return a < b? b : a;
}

template <>
cv::Point2f findMaxInArrayHelper(const cv::Point2f& a, const cv::Point2f & b){
	return cv::Point2f(findMaxInArrayHelper(a.x,b.x),findMaxInArrayHelper(a.y,b.y));
}

template<typename T, int N>
T findMaxInArray(const std::array<T,N> & array){
	static_assert(N>0, "N has to be larger than zero");
	T max = array[0];
	for (int i=1; i<N; i++){
		max = findMaxInArrayHelper(max, array[i]);
	}
	return max;
}

template <typename T>
T findMinInArrayHelper(const T & a, const T & b){
    static_assert(std::is_arithmetic<T>::value, "Not Implemented findMinInArrayHelper");
  return a < b ? a : b;
}

template <>
cv::Point2f findMinInArrayHelper(const cv::Point2f& a, const cv::Point2f & b){
    return cv::Point2f(findMinInArrayHelper(a.x,b.x),findMinInArrayHelper(a.y,b.y));
}

template<typename T, int N>
T findMinInArray(const std::array<T,N> &array)
{
    static_assert(N>0, "N has to be larger than zero");
    T min = array[0];
    for(auto element : array)
    {
        min = findMinInArrayHelper(min, element);
    }
    return min;
}

template<int N>
cv::Point2i calculateImageSize(std::array<cv::Point2f, N> landmarks)
{
    return findMaxInArray<cv::Point2f, N>(landmarks) - findMinInArray<cv::Point2f, N>(landmarks);
}

#endif
