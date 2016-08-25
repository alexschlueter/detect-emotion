#include "exporter.h"
#include <numeric>
#include <cmath>
#include <fstream>

PythonExport::PythonExport(string filename):_filename(filename){
    python_text << "import numpy as np\nfrom matplotlib import pyplot as plt" << endl << endl;
    test_res << "test_res = {" ;
    train_res << "train_res = {";
}

void PythonExport::writeResult(){
    test_res << "\n}";
    train_res << "\n}";
    python_text << test_res.str()<<endl<<endl<<train_res.str()<<endl;
}

#define checkInvalid(n) if (n == numeric_limits<double>::infinity() || isnan(n)) n = 0

void PythonExport::addTestResult(double recall, double precision, const string & name){
   checkInvalid(recall);
   checkInvalid(precision);
   test_res<<"\n\t\""<<name<<"\": {\"precision\":"<<precision<<", \"recall\": "<<recall<<"},";
}

void PythonExport::addTrainResult(double recall, double precision, const string & name){
   checkInvalid(recall);
   checkInvalid(precision);
   train_res<<"\n\t\""<<name<<"\": {\"precision\":"<<precision<<", \"recall\": "<<recall<<"},";
}


void PythonExport::writePlot(){
    python_text << "def plotIt(tset, name, ax):\n";
    python_text << "\tprecisions = np.array([tset[k]['precision'] for k in tset])\n";
    python_text << "\trecalls = np.array([tset[k]['recall'] for k in tset])\n";
    python_text << "\tlabels = tset.keys()\n";
    python_text << "\tax.plot(precisions, recalls,'pr')\n";
    python_text << "\tfor i in xrange(len(labels)):\n";
    python_text << "\t\tp,r = precisions[i], recalls[i]\n";
    python_text << "\t\tax.annotate(labels[i],"\
                     "xy=(p,r), xytext=(p,r+0.05), "\
                     "arrowprops=dict(facecolor='black', shrink=0.005), )\n";
    python_text << "\tax.set_ylim(0,1.02)\n";
    python_text << "\tax.set_xlim(0,1)\n";
    python_text << "\tax.set_title(name)\n";
    python_text << "\tax.set_xlabel('precision')\n";
    python_text << "\tax.set_ylabel('recall')\n";
    python_text << "fig = plt.figure()\n";
    python_text << "ax = fig.add_subplot(211)\n";
    python_text << "plotIt(test_res, 'test-result',ax)\n";
    python_text << "ax = fig.add_subplot(212)\n";
    python_text << "plotIt(train_res, 'train-result',ax)\n";
    python_text << "plt.show()\n";
}
void PythonExport::save(const string & filename){
    writeResult();
    writePlot();
    string text = this->text();
    ofstream file(filename);
    file << text;
    file.close();
}
