#pragma once
#include <vector>
#include <string>
#include <map>
#include "Assessment.h"
#include "Course.h"
#include "Student.h"
using namespace std;

class GradeEngine {
public:
    // ── Core ──────────────────────────────────────────────────────
    static double calcAbsolute(double obtained, double total, double weightage);
    static double calcRunningAbsolute(const vector<Assessment>& marks);
    static double calcRemainingWeightage(const vector<Assessment>& marks);
    // Running score normalized to the weightage actually tested so far (0-100).
    // Use this (not calcRunningAbsolute) for risk/color thresholds — the raw
    // running total is capped at (100 - remaining), so it looks artificially
    // low early in the semester even for a strong student.
    static double calcPace(const vector<Assessment>& marks);

    // ── Grading ───────────────────────────────────────────────────
    // Relative grade using normal distribution with customisable SD thresholds
    static string calcRelativeGrade(double studentAbs, double classMeanAbs,
                                     double classSD, const Student& s);
    // Picks absolute or relative depending on student.gradingMode
    static string getCourseGrade(const Course& c, const Student& s, bool isLab = false);

    // ── GPA ────────────────────────────────────────────────────────
    static double calcCourseSGPA(const Course& c, const Student& s);
    static double calcSemesterSGPA(const vector<Course>& courses, const Student& s);
    static double calcProjectedCGPA(const Student& s, double newSGPA, int semCredits);
    static double calcRequiredSGPA(const Student& s, double targetCGPA);
    static int    totalSemCredits(const vector<Course>& courses);

    // ── Prediction ─────────────────────────────────────────────────
    static string predictGrade(double absolute, const Student& s);
    static string calcBestCase(const Course& c, const Student& s, bool isLab = false);
    static string calcWorstCase(const Course& c, const Student& s, bool isLab = false);

    // ── Gap analysis ───────────────────────────────────────────────
    static double calcRequiredInRemaining(const Course& c, const string& targetGrade,
                                          const Student& s, bool isLab = false);

    // ── Statistics ─────────────────────────────────────────────────
    static Assessment calcClassStats(const vector<double>& allMarks,
                                     double studentMark,
                                     const Assessment& original);

    // ── Weightage helpers ──────────────────────────────────────────
    static void redistributeWeightage(vector<Assessment>& marks,
                                      const string& type, double categoryTotal);

    // ── Minimum to pass ────────────────────────────────────────────
    static double calcMinToPass(const Course& c, const Student& s,
                                const string& minGrade = "D", bool isLab = false);

    // ── New features ───────────────────────────────────────────────
    static string calcTrend(const vector<Assessment>& marks, const string& type);
    static double calcPriorityScore(const Course& c, const Student& s);
    static map<string,string> suggestCourseGrades(const Student& s,
                                                   const vector<Course>& courses,
                                                   double targetCGPA);
};
