%module detectVanishingP
%{
    #include "detectVP.h"
    using namespace cv;
    using namespace std;

    extern "C" int detectVanishingPoint(char* dirAndName,char* imgName);
%}

int detectVanishingPoint(char* dirAndName,char* imgName);

