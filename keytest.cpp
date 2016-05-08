#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;

int main() {
    cv::namedWindow("Image", CV_WINDOW_NORMAL);
    cout << cv::waitKey(0);
}
