#include "util.h"
#include "actionunit.h"
#include "reader.h"
#include <QtCore>

void mkdirs(const string &filename){
    QString qfilename = QString::fromStdString(filename);
    QDir dir;
    if (qfilename.endsWith("/")){
        dir = QDir(qfilename);
    }else{
        QFileInfo info(qfilename);
        dir = info.dir();
    }
    if (dir.exists()) return;
    dir.mkpath(".");
}


string dirOfFile(const string &filename){
    QFileInfo dir(QString::fromStdString(filename));
    return dir.path().toStdString();
}

Video loadLandmark(const std::string &filename){
    auto landm =  readBinaryFile<66>(filename);
    std::vector<PointCloud<66>> res;
    res.reserve(landm.size());
    for (auto el: landm){
        res.emplace_back(el);
    }
    return res;
}

VideoList loadLandmarkFromFolder(const string & dir){
    QDir d(QString::fromStdString(dir));
    d.setNameFilters(QStringList()<<"*.bin");
    auto entries = d.entryInfoList();
    VideoList res;
    res.reserve(entries.size());
    for (auto && filename: entries){
        res.push_back(loadLandmark(filename.absoluteFilePath().toStdString()));
    }
    return res;
}

vector<ActionUnit> loadActionUnitFormFolder(const string & dir){
    QDir d(QString::fromStdString(dir));
    auto subdirs = d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    vector<ActionUnit> res;
    res.reserve(subdirs.size());
    for (auto && subdir : subdirs){
        auto filename = QStringLiteral("%1/%2_AUs.txt").arg(subdir.absoluteFilePath()).arg(QDir(subdir.absoluteFilePath()).dirName());
        res.emplace_back(readActionUnitFromFile(filename.toStdString()));
    }
    return res;
}
