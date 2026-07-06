#include "GradeEngine.h"
#include <algorithm>
#include <numeric>
#include <cmath>
using namespace std;

double GradeEngine::calcAbsolute(double obtained, double total, double weightage) {
    if (total <= 0) return 0;
    return (obtained / total) * weightage;
}
double GradeEngine::calcRunningAbsolute(const vector<Assessment>& marks) {
    double t = 0;
    for (auto& a : marks) t += a.absolute;
    return t;
}
double GradeEngine::calcRemainingWeightage(const vector<Assessment>& marks) {
    double used = 0;
    for (auto& a : marks) used += a.weightage;
    return max(0.0, 100.0 - used);
}
double GradeEngine::calcPace(const vector<Assessment>& marks) {
    double running = calcRunningAbsolute(marks);
    double tested   = 100.0 - calcRemainingWeightage(marks);
    return (tested > 0) ? (running / tested) * 100.0 : 0.0;
}
int GradeEngine::totalSemCredits(const vector<Course>& courses) {
    int t = 0;
    for (auto& c : courses) t += c.theoryCredits + c.labCredits;
    return t;
}

// ── Relative grading ──────────────────────────────────────────────────────────
// Uses student.relativeThresholds: A=+1.5SD, B=+0.5SD, C=-0.5SD, D=-1.5SD
// Sub-bands within each zone map to +/- / plain letter
string GradeEngine::calcRelativeGrade(double studentAbs, double classMeanAbs,
                                       double classSD, const Student& s) {
    if (classSD <= 0) return s.getGrade(studentAbs); // no spread, fall back

    // Get customisable thresholds (with defaults if missing)
    auto get = [&](const string& k, double def) -> double {
        auto it = s.relativeThresholds.find(k);
        return (it != s.relativeThresholds.end()) ? it->second : def;
    };
    double thA = get("A", 1.5);
    double thB = get("B", 0.5);
    double thC = get("C", -0.5);
    double thD = get("D", -1.5);

    double z = (studentAbs - classMeanAbs) / classSD;

    // Map z to grade with sub-bands
    if      (z >= thA + 0.5)  return "A+";
    else if (z >= thA)         return "A";
    else if (z >= thA - 0.25) return "A-";
    else if (z >= thB + 0.25) return "B+";
    else if (z >= thB)         return "B";
    else if (z >= thB - 0.25) return "B-";
    else if (z >= thC + 0.25) return "C+";
    else if (z >= thC)         return "C";
    else if (z >= thD)         return "D";
    else                       return "F";
}

// Picks absolute or relative depending on student.gradingMode
string GradeEngine::getCourseGrade(const Course& c, const Student& s, bool isLab) {
    const auto& marks = isLab ? c.labMarks : c.theoryMarks;
    if (marks.empty()) return "N/A";

    double running = calcRunningAbsolute(marks);

    if (s.gradingMode == "relative") {
        // Aggregate mean and SD from stored class stats across all assessments
        double meanSum = 0, sdSum = 0;
        int count = 0;
        for (auto& a : marks) {
            if (a.hasClassStats && a.total > 0) {
                // Normalise class stats to % of total weightage contribution
                double scale = a.weightage / a.total;
                meanSum += a.classMean * scale;
                sdSum   += a.classStdDev * scale;
                count++;
            }
        }
        if (count > 0) {
            double avgMean = meanSum; // already scaled to absolute %
            double avgSD   = sdSum;
            return calcRelativeGrade(running, avgMean, avgSD, s);
        }
        // Not enough class data yet — fall back to absolute with a note
        return s.getGrade(running);
    }
    return s.getGrade(running);
}

// ── GPA ───────────────────────────────────────────────────────────────────────
double GradeEngine::calcCourseSGPA(const Course& c, const Student& s) {
    double totalCredits = c.theoryCredits + c.labCredits;
    if (totalCredits == 0) return 0;
    double ws = 0;
    if (!c.isLabOnly()) {
        ws += s.getPoints(getCourseGrade(c, s, false)) * c.theoryCredits;
    }
    if (c.hasLab && c.labCredits > 0) {
        ws += s.getPoints(getCourseGrade(c, s, true)) * c.labCredits;
    }
    return ws / totalCredits;
}

double GradeEngine::calcSemesterSGPA(const vector<Course>& courses, const Student& s) {
    double totalQP = 0, totalCreds = 0;
    for (auto& c : courses) {
        if (c.theoryMarks.empty() && c.labMarks.empty()) continue;
        if (!c.isLabOnly()) {
            totalQP    += s.getPoints(getCourseGrade(c,s,false)) * c.theoryCredits;
            totalCreds += c.theoryCredits;
        }
        if (c.hasLab && c.labCredits > 0) {
            totalQP    += s.getPoints(getCourseGrade(c,s,true)) * c.labCredits;
            totalCreds += c.labCredits;
        }
    }
    return (totalCreds > 0) ? totalQP / totalCreds : 0;
}

double GradeEngine::calcProjectedCGPA(const Student& s, double newSGPA, int semCredits) {
    double totalQP = s.previousCGPA * s.previousCredits + newSGPA * semCredits;
    double totalC  = s.previousCredits + semCredits;
    return (totalC > 0) ? totalQP / totalC : newSGPA;
}

double GradeEngine::calcRequiredSGPA(const Student& s, double targetCGPA) {
    int est = 18;
    double total = s.previousCredits + est;
    double req   = (targetCGPA * total - s.previousCGPA * s.previousCredits) / est;
    return max(0.0, min(4.0, req));
}

// ── Grade prediction ──────────────────────────────────────────────────────────
string GradeEngine::predictGrade(double absolute, const Student& s) {
    return s.getGrade(absolute);
}
string GradeEngine::calcBestCase(const Course& c, const Student& s, bool isLab) {
    const auto& m = isLab ? c.labMarks : c.theoryMarks;
    return s.getGrade(calcRunningAbsolute(m) + calcRemainingWeightage(m));
}
string GradeEngine::calcWorstCase(const Course& c, const Student& s, bool isLab) {
    const auto& m = isLab ? c.labMarks : c.theoryMarks;
    return s.getGrade(calcRunningAbsolute(m));
}

// ── Gap analysis ──────────────────────────────────────────────────────────────
double GradeEngine::calcRequiredInRemaining(const Course& c, const string& targetGrade,
                                             const Student& s, bool isLab) {
    const auto& m = isLab ? c.labMarks : c.theoryMarks;
    double running  = calcRunningAbsolute(m);
    double remaining= calcRemainingWeightage(m);
    auto it = s.gradeBoundaries.find(targetGrade);
    if (it == s.gradeBoundaries.end()) return 0;
    if (remaining <= 0) return -1;
    double needed = it->second - running;
    if (needed <= 0) return 0;
    return min(100.0, max(0.0, (needed / remaining) * 100.0));
}

// ── Statistics ────────────────────────────────────────────────────────────────
Assessment GradeEngine::calcClassStats(const vector<double>& allMarks,
                                        double studentMark,
                                        const Assessment& original) {
    Assessment a = original;
    if (allMarks.empty()) return a;
    vector<double> sorted = allMarks;
    sort(sorted.begin(), sorted.end());
    int n = (int)sorted.size();
    a.classSize = n;
    double sum = accumulate(allMarks.begin(), allMarks.end(), 0.0);
    a.classMean = sum / n;
    a.classMedian = (n%2==0) ? (sorted[n/2-1]+sorted[n/2])/2.0 : sorted[n/2];
    double sq = 0;
    for (double v : allMarks) sq += (v-a.classMean)*(v-a.classMean);
    a.classVariance = sq/n;
    a.classStdDev   = sqrt(a.classVariance);
    a.classTopper   = sorted.back();
    a.classLowest   = sorted.front();
    int rank = 1;
    for (double v : allMarks) if (v > studentMark) rank++;
    a.studentRank = rank;
    int below = (int)count_if(allMarks.begin(),allMarks.end(),[&](double v){return v<studentMark;});
    a.studentPercentile = (n>1) ? 100.0*below/(n-1) : 100.0;
    a.marksAboveMean    = studentMark - a.classMean;
    a.hasClassStats     = true;
    return a;
}

// ── Weightage redistribution ──────────────────────────────────────────────────
void GradeEngine::redistributeWeightage(vector<Assessment>& marks,
                                         const string& type, double categoryTotal) {
    int count = 0;
    for (auto& a : marks) if (a.type==type) count++;
    if (count==0) return;
    double per = categoryTotal / count;
    for (auto& a : marks) if (a.type==type) {
        a.weightage = per;
        a.absolute  = calcAbsolute(a.obtained, a.total, a.weightage);
    }
}

// ── Minimum to pass ───────────────────────────────────────────────────────────
double GradeEngine::calcMinToPass(const Course& c, const Student& s,
                                   const string& minGrade, bool isLab) {
    return calcRequiredInRemaining(c, minGrade, s, isLab);
}

// ── Trend ─────────────────────────────────────────────────────────────────────
string GradeEngine::calcTrend(const vector<Assessment>& marks, const string& type) {
    vector<double> eff;
    for (auto& a : marks)
        if (a.type==type && a.total>0) eff.push_back(a.obtained/a.total);
    if (eff.size() < 2) return "N/A";
    int n=(int)eff.size(), half=n/2;
    double first=0, second=0;
    for (int i=0;i<half;i++)  first  += eff[i];
    for (int i=half;i<n;i++)  second += eff[i];
    first /= half; second /= (n-half);
    double diff = second - first;
    if      (diff >  0.05) return "Improving";
    else if (diff < -0.05) return "Declining";
    else                   return "Stable";
}

// ── Priority score ────────────────────────────────────────────────────────────
double GradeEngine::calcPriorityScore(const Course& c, const Student& s) {
    const auto& m = c.isLabOnly() ? c.labMarks : c.theoryMarks;
    double running   = calcRunningAbsolute(m);
    double remaining = calcRemainingWeightage(m);
    auto it = s.gradeBoundaries.find(c.targetGrade);
    double targetB = (it != s.gradeBoundaries.end()) ? it->second : 75.0;
    double gap     = max(0.0, targetB - running);
    double rankFactor = 0;
    if (!m.empty() && m.back().hasClassStats && m.back().classSize > 0)
        rankFactor = 100.0 - m.back().studentPercentile;
    return (gap * 0.5) + (remaining * 0.3) + (rankFactor * 0.2);
}

// ── Suggest grades per course ─────────────────────────────────────────────────
map<string,string> GradeEngine::suggestCourseGrades(const Student& s,
                                                      const vector<Course>& courses,
                                                      double targetCGPA) {
    map<string,string> sug;
    double req = calcRequiredSGPA(s, targetCGPA);
    for (auto& c : courses) {
        int creds = c.theoryCredits + c.labCredits;
        string grade;
        if      (req >= 3.7) grade = (creds>=3) ? "A"  : "A-";
        else if (req >= 3.3) grade = (creds>=3) ? "A-" : "B+";
        else if (req >= 3.0) grade = (creds>=3) ? "B+" : "B";
        else if (req >= 2.7) grade = "B";
        else if (req >= 2.3) grade = "B-";
        else if (req >= 2.0) grade = "C+";
        else                 grade = "C";
        sug[c.name] = grade;
    }
    return sug;
}
