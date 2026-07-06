#pragma once
#include <string>
#include <vector>
#include "Assessment.h"
using namespace std;

struct Course {
    string name;
    string shortCode;

    int  theoryCredits = 3;
    int  labCredits    = 0;
    bool hasLab        = false;

    // Theory weightages (must sum to 100)
    double quizW       = 0;
    double assignmentW = 0;
    double midW        = 0;
    double finalW      = 50;
    double projectW    = 0;
    bool   hasProject  = false;

    // Lab weightages (sum to 100 independently)
    double practicalW    = 0;
    double labMidW       = 0;
    double labFinalW     = 0;
    double labProjectW   = 0;
    bool   labHasProject = false;

    // Counts
    int  quizCount       = 0;
    int  assignmentCount = 0;
    int  practicalCount  = 0;

    // Drop lowest
    bool dropLowestQuiz      = false;
    bool dropLowestPractical = false;

    // Personal target
    string targetGrade = "B+";

    // Marks
    vector<Assessment> theoryMarks;
    vector<Assessment> labMarks;

    // Attendance
    int attended     = 0;
    int totalClasses = 0;

    // Deadline calendar: { "Quiz 1", "2026-05-10" }
    vector<pair<string,string>> upcomingDates;

    // ── helpers ──────────────────────────────────────────────────
    int    nextNumber(const string& type, bool isLab = false) const;
    double runningAbsolute(bool isLab = false) const;
    double remainingWeightage(bool isLab = false) const;
    bool   isLabOnly() const;
};
