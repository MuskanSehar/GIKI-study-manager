#pragma once
#include "Student.h"
#include "Course.h"
#include "GradeEngine.h"
using namespace std;

class Dashboard {
public:
    static void showMainMenu(const Student& s, const vector<Course>& courses);
    static void showHealthSnapshot(const Student& s, const vector<Course>& courses);
    static void showSubjectDashboard(const Course& c, const Student& s);
    static void showSemesterSummary(const vector<Course>& courses, const Student& s);
    static void showGradePredictions(const vector<Course>& courses, const Student& s);
    static void showOverallAnalysis(const vector<Course>& courses, const Student& s);
    static void showWhatIf(const vector<Course>& courses, const Student& s);
    static void showMinToPass(const vector<Course>& courses, const Student& s);
    static void showAttendance(const vector<Course>& courses);
    static void showHistory(const Student& s);
    static void showTargetCGPA(const Student& s, const vector<Course>& courses);

    // New features
    static void showPriorityList(const vector<Course>& courses, const Student& s);
    static void showDeadlineCalendar(const vector<Course>& courses);
    static void showExportSummary(const Student& s, const vector<Course>& courses);
    static void showCreditSummary(const vector<Course>& courses);

    static void clearScreen();
    static void printLine(char c = '-', int width = 62);
    static void printHeader(const string& title);
    static void pause();

    static const string RESET, BOLD, RED, GREEN, YELLOW, CYAN, MAGENTA, WHITE, DIM;
};
