#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include <pointcloud.h>
#include <actionunit.h>

#if defined __unix__ || defined __APPLE__ || defined __FreeBSD__ || defined __linux__
#define onunix 1
#else
#define onunix 0
#endif

using namespace std;

using Video = std::vector<PointCloud<66>>;
using VideoList = std::vector<Video>;

VideoList loadLandmarkFromFolder(const string & dir);
vector<ActionUnit> loadActionUnitFormFolder(const string & dir);


void mkdirs(const string & filename);
string dirOfFile(const string & filename);


#endif // UTIL_H
