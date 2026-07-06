#pragma once
#include <string>
#include <vector>
#include "Student.h"
#include "Course.h"
using namespace std;

class FileManager {
public:
    // ── Startup ────────────────────────────────────────────────────
    static void createFolderStructure();
    static bool profileExists();

    // ── Student profile ────────────────────────────────────────────
    static bool saveProfile(const Student& s);
    static bool loadProfile(Student& s);

    // ── Courses ────────────────────────────────────────────────────
    static bool saveCourses(const vector<Course>& courses);
    static bool loadCourses(vector<Course>& courses);

    // ── Marks ──────────────────────────────────────────────────────
    static bool saveMarks(const Course& c);
    static bool loadMarks(Course& c);

    // ── Buffer file ────────────────────────────────────────────────
    // Returns { {regno, marks} } — empty on error
    static vector<pair<string,double>> readBufferFile(string& errorMsg);
    static void wipeBufferFile();

    // ── History ────────────────────────────────────────────────────
    static bool appendHistory(const SemesterRecord& rec);
    static bool loadHistory(vector<SemesterRecord>& history);

    // ── Report ─────────────────────────────────────────────────────
    static bool generateReport(const Student& s, const vector<Course>& courses);

    // ── Path helpers ───────────────────────────────────────────────
    static string theoryPath(const string& code);
    static string labPath(const string& code);
    static string bufferPath();
    static string profilePath();
    static string semesterPath();
    static string historyPath();

private:
    static string sanitize(const string& name);
    static string todayString();
};
