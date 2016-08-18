#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include <pointcloud.h>
#include <actionunit.h>
using namespace std;

using Video = std::vector<PointCloud<66>>;
using VideoList = std::vector<Video>;

VideoList loadLandmarkFromFolder(const string & dir);
vector<ActionUnit> loadActionUnitFormFolder(const string & dir);


void mkdirs(const string & filename);
string dirOfFile(const string & filename);

#endif // UTIL_H
