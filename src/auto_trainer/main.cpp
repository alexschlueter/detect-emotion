#include <iostream>
#include "train.h"
#include "eval.h"
using namespace std;

void printHelp(char* name){
    cout << name<< " [train|eval] "<< endl;
}

int main(int argc, char** argv){
 if (argc < 2) {
     printHelp(argv[0]);
     return EXIT_FAILURE;
 }
 if (strcmp(argv[1], "train")==0){
     return trainmain(argv[0],argc-2,argv+2);
 }else if (strcmp(argv[1],"eval")==0){
     return evalmain(argv[0],argc-2,argv+2);
 }else{
     printHelp(argv[0]);
     return EXIT_FAILURE;
 }
}
