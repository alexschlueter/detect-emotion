#ifndef EVALWRITER_H
#define EVALWRITER_H
#include <string>
#include "classifier.h"
#include "wrapper.h"
#include "processors.h"
#include "trainparser.h"
using namespace std;
string serializeEvalConfig(
        const string & actionname,
        const string & cloud_save_dir,
        const vector<CloudProcessorP> & cloud_processors,
        const ClassifierConstructor & classifier,
        const string & classifier_save_filename,
        int action_threshold,
        const string & processor_save_dir,
        const FrameFeatureProcessor * frameFeature,
        const TimeFeatureProcessor * timeFeature,
        int time_frame_step);

#endif // EVALWRITER_H
