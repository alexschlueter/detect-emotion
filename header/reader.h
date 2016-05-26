#ifndef READER_H
#define READER_H
#include <fstream>
#include <vector>
#include <array>
#include <opencv2/core/core.hpp>
#include "myendian.h"

/**
 * Reads a binary landmark file.
 * N is the number of landmarks for a frame.
 * @param inputstream opened stream for reading the binary file
 * @return vector containing an array landmark for every frame
 *         zero-length vector on fail.
 */
template <int N=66>
std::vector<std::array<cv::Point2f, N>> readBinaryFile(std::istream & inputstream){
   std::vector<std::array<cv::Point2f,N>> result;
   if (!inputstream.good()) return result;
   bool cont = true;
   while (cont){
    char bytes[2*sizeof(float)];
    std::array<cv::Point2f, N> frame;
    for (int i=0; i< N; i++){
      if (!inputstream.read(bytes,2*sizeof(float))){
        cont = false;
        break;
      }
      float a = *(reinterpret_cast<float*>(bytes));
      float b = *(reinterpret_cast<float*>(bytes+sizeof(float)));
      toEndianType<float, LITTLENDIAN>(&a, &a);
      toEndianType<float, LITTLENDIAN>(&b, &b);
      frame[i].x = a;
      frame[i].y = b;
    }
    result.push_back(frame);
   }
   return result;
}

/**
 * Reads a binary landmark file.
 * N is the number of landmarks for a frame.
 * @param filename filname of the binary landmark file
 * @return vector containing an array landmark for every frame
 *         zero-length vector on fail.
 */
template <int N=66>
std::vector<std::array<cv::Point2f, N>> readBinaryFile(const std::string & filename){
    std::ifstream stream(filename,std::ios::binary);
    auto res = readBinaryFile<N>(stream);
    stream.close();
    return res;
}

#endif
