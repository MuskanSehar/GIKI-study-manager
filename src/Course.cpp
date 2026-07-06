#include "Course.h"
#include <algorithm>
using namespace std;

int Course::nextNumber(const string& type, bool isLab) const {
    const auto& marks = isLab ? labMarks : theoryMarks;
    int count = 0;
    for (auto& a : marks) if (a.type == type) count++;
    return count + 1;
}

double Course::runningAbsolute(bool isLab) const {
    const auto& marks = isLab ? labMarks : theoryMarks;
    double total = 0;
    for (auto& a : marks) total += a.absolute;
    return total;
}

double Course::remainingWeightage(bool isLab) const {
    const auto& marks = isLab ? labMarks : theoryMarks;
    double used = 0;
    for (auto& a : marks) used += a.weightage;
    return max(0.0, 100.0 - used);
}

bool Course::isLabOnly() const {
    return theoryCredits == 0 && labCredits > 0;
}
