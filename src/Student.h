#pragma once
#include <string>
#include <vector>
#include <map>
#include "Assessment.h"
using namespace std;

struct Course;

struct SemesterRecord {
    string sessionLabel;
    int    semesterNumber = 0;
    double sgpa           = 0;
    int    credits        = 0;
};

class Student {
public:
    string regno;
    string name;
    string sessionLabel;
    int    currentSemester = 1;
    double previousCGPA    = 0.0;
    int    previousCredits = 0;

    // ── Grading mode (per semester) ───────────────────────────────
    string gradingMode = "absolute"; // "absolute" or "relative"

    // Relative grading SD thresholds (customisable)
    // grade = mean + threshold*SD
    // defaults: A=+1.5, B=+0.5, C=-0.5, D=-1.5, F=below
    map<string,double> relativeThresholds = {
        {"A", 1.5}, {"B", 0.5}, {"C", -0.5}, {"D", -1.5}
    };

    // Teacher average for the semester (used in relative mode)
    // -1 = not set yet
    double teacherAvg = -1;

    // Absolute grade boundaries
    map<string,double> gradeBoundaries = {
        {"A+",90},{"A",85},{"A-",80},
        {"B+",75},{"B",70},{"B-",65},
        {"C+",60},{"C",55},{"C-",50},
        {"D",45},{"F",0}
    };

    map<string,double> gradePoints = {
        {"A+",4.0},{"A",4.0},{"A-",3.7},
        {"B+",3.3},{"B",3.0},{"B-",2.7},
        {"C+",2.3},{"C",2.0},{"C-",1.7},
        {"D",1.0},{"F",0.0}
    };

    vector<SemesterRecord> history;

    string getGrade(double absolute) const;
    double getPoints(const string& grade) const;
};
