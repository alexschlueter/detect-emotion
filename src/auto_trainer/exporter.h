#ifndef EXPORTER_H
#include <string>
#include <sstream>

using namespace std;

struct PythonExport{
    PythonExport(string filename);
    void addTestResult(double recall, double precision, const string & name);
    void addTrainResult(double recall, double precision, const string & name);
    void writeResult();
    void writePlot();
    inline string text() const { return python_text.str();}
    stringstream python_text;
    stringstream test_res;
    stringstream train_res;
    void save(const string & filename);
    inline void save(){
        save(_filename);
    }

    string _filename;
};
#define EXPORTER_H
#endif // EXPORTER_H
