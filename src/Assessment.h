#pragma once
#include <string>
using namespace std;

struct Assessment {
    string type;        // Quiz / Assignment / Mid / Final / Project / Practical
    int    number;      // auto-incremented: 1, 2, 3...
    double obtained;    // marks student got
    double total;       // assessment was out of this
    double weightage;   // % this contributes to final grade
    double absolute;    // = (obtained / total) * weightage

    // Class statistics — stored permanently after buffer file read
    double classMean        = 0;
    double classMedian      = 0;
    double classStdDev      = 0;
    double classVariance    = 0;
    double classTopper      = 0;
    double classLowest      = 0;
    int    studentRank      = 0;
    int    classSize        = 0;
    double studentPercentile= 0;
    double marksAboveMean   = 0;
    bool   hasClassStats    = false;

    Assessment() : number(0), obtained(0), total(0), weightage(0), absolute(0) {}

    Assessment(string t, int n, double obt, double tot, double w)
        : type(t), number(n), obtained(obt), total(tot), weightage(w) {
        absolute = (tot > 0) ? (obt / tot) * w : 0;
    }
};
