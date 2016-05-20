#ifndef UTILS_H
#define UTILS_H

#include <array>
#include <type_traits>
#include <opencv2/core/core.hpp>

template <typename T>
T findMaxInArrayHelper(const T & a, const T & b){
	static_assert(!std::is_arithmetic<T>::value, "Not Implemented findMaxInArrayHelper");
}

template <>
cv::Point2f findMaxInArrayHelper(const cv::Point2f& a, const cv::Point2f & b){
	return cv::Point2f(std::max(a.x,b.x),std::max(a.y,b.y));
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

#endif 