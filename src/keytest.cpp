#include <iostream>
#include "opencv2/opencv.hpp"
#include <bitset>

using namespace std;

int main() {
    cv::namedWindow("Image", CV_WINDOW_NORMAL);
    for (;;) {
        int k = cv::waitKey(0);
        if (k == -1) break;
        cout << bitset<32>(k) << " " << k << " " << (k & 0xFF) << " '" << (char) (k & 0xFF) << "'" << endl;
    }
}
