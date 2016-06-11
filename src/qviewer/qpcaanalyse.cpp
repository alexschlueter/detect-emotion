#include "qpcaanalyse.h"
#include <QVariant>
#include "qlandmark.h"
#include "pointcloud.h"
#include <QDebug>

void QPCAAnalyse::analyseData(const QVariantList &clouds){
    std::vector<PointCloud<66>> pointcloud;
    auto extractCloudList = [&pointcloud](const QVariantList & c){
        for(auto & cloud: c){
            if (!cloud.canConvert<QLandmark*>()) return false;
            QLandmark* l = cloud.value<QLandmark*>();
            pointcloud.push_back(l->pointCloud());
        }
        return true;
    };
    if (clouds.isEmpty()) return;
    if (clouds[0].canConvert<QVariantList>()){
        for (int i=0; i<clouds.length(); i++){
            if (!extractCloudList(clouds[i].toList())) return;
        }
    }else {
        if (!extractCloudList(clouds)) return;
    }
    this->pca = std::unique_ptr<PCA_Result<66>>(new PCA_Result<66>(computePCA(pointcloud)));
}

QPCAAnalyse::QPCAAnalyse(const QVariantList & clouds, QObject* parent): QObject(parent) {
    analyseData(clouds);
}


QVariantList QPCAAnalyse::rebuildPoints(QLandmark  * landmark, int dim){
    if (isEmpty()){
        qDebug() << "QPCAAnalyse: RebuildPoints is empty";
        return QVariantList();
    }
    if (landmark == nullptr){
        qDebug() << "QPCAAnalyse: landmark is null";
        return QVariantList();
    }
    if (dim > 66){
        qDebug() << "QPCAAnalyse: dim "<<dim<< " is greater than 66. Set it to 66.";
        dim = 66;
    }
    return  pointToVarList<66>(pca->rebuild(landmark->pointCloud(),dim).points());

}
