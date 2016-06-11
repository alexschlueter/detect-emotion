#ifndef QPCAANALYSE_H
#define QPCAANALYSE_H

#include <QObject>
#include <pca.h>
#include <QVariantList>
#include <qqml.h>
class QLandmark;

class QPCAAnalyse:public QObject
{
    Q_OBJECT
public:

    QPCAAnalyse(const QVariantList & clouds, QObject* parent=nullptr);

    explicit QPCAAnalyse(QObject* parent=nullptr): QObject(parent){}

    Q_INVOKABLE void analyseData(const QVariantList & clouds);

    Q_INVOKABLE bool isEmpty() const{
        return pca == nullptr;
    }

    Q_INVOKABLE QVariantList rebuildPoints(QLandmark* landmarks, int dim);

private:
    std::unique_ptr<PCA_Result<66>> pca;
};

#endif // QPCAANALYSE_H
