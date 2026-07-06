#include "Student.h"
#include <string>
using namespace std;

string Student::getGrade(double absolute) const {
    // Walk boundaries from highest to lowest
    const vector<pair<string,double>> order = {
        {"A+",90},{"A",85},{"A-",80},
        {"B+",75},{"B",70},{"B-",65},
        {"C+",60},{"C",55},{"C-",50},
        {"D",45}
    };
    for (auto& [grade, cutoff] : order) {
        auto it = gradeBoundaries.find(grade);
        double cut = (it != gradeBoundaries.end()) ? it->second : cutoff;
        if (absolute >= cut) return grade;
    }
    return "F";
}

double Student::getPoints(const string& grade) const {
    auto it = gradePoints.find(grade);
    return (it != gradePoints.end()) ? it->second : 0.0;
}
