#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <ctime>
#include <algorithm>
using namespace std;
namespace fs = filesystem;

// ── Path helpers ──────────────────────────────────────────────────────────────
string FileManager::sanitize(const string& name) {
    string s = name;
    for (char& c : s) if (c==' '||c=='/'||c=='\\'||c=='\''||c=='"') c='_';
    return s;
}

string FileManager::todayString() {
    time_t t = time(nullptr);
    tm* tm_info = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%b%d", tm_info);
    return string(buf);
}

string FileManager::profilePath()   { return "data/student_profile.txt"; }
string FileManager::semesterPath()  { return "data/current_semester.txt"; }
string FileManager::historyPath()   { return "data/semester_history.txt"; }
string FileManager::bufferPath()    { return "inbox/input.txt"; }
string FileManager::theoryPath(const string& code) {
    return "data/results/" + sanitize(code) + "_theory.csv";
}
string FileManager::labPath(const string& code) {
    return "data/results/" + sanitize(code) + "_lab.csv";
}

// ── Folder structure ──────────────────────────────────────────────────────────
void FileManager::createFolderStructure() {
    fs::create_directories("data/results");
    fs::create_directories("inbox");
    fs::create_directories("reports");

    // Ensure buffer file exists
    if (!fs::exists(bufferPath())) {
        ofstream f(bufferPath());
        f << "# Format: regno,marks\n";
    }
}

bool FileManager::profileExists() {
    return fs::exists(profilePath());
}

// ── Student profile ───────────────────────────────────────────────────────────
bool FileManager::saveProfile(const Student& s) {
    ofstream f(profilePath());
    if (!f.is_open()) return false;
    f << "regno=" << s.regno << "\n";
    f << "name=" << s.name << "\n";
    f << "session=" << s.sessionLabel << "\n";
    f << "semester=" << s.currentSemester << "\n";
    f << "prevCGPA=" << s.previousCGPA << "\n";
    f << "prevCredits=" << s.previousCredits << "\n";
    f << "gradingMode=" << s.gradingMode << "\n";
    f << "teacherAvg=" << s.teacherAvg << "\n";
    for (auto& [g,t] : s.relativeThresholds)
        f << "relThresh_" << g << "=" << t << "\n";
    // Grade boundaries
    for (auto& [g, cut] : s.gradeBoundaries)
        f << "boundary_" << g << "=" << cut << "\n";
    return true;
}

bool FileManager::loadProfile(Student& s) {
    ifstream f(profilePath());
    if (!f.is_open()) return false;
    string line;
    while (getline(f, line)) {
        if (line.empty() || line[0]=='#') continue;
        auto eq = line.find('=');
        if (eq == string::npos) continue;
        string key = line.substr(0, eq);
        string val = line.substr(eq + 1);
        if (key=="regno")       s.regno           = val;
        else if (key=="name")   s.name            = val;
        else if (key=="session")s.sessionLabel    = val;
        else if (key=="semester")    s.currentSemester = stoi(val);
        else if (key=="prevCGPA")    s.previousCGPA    = stod(val);
        else if (key=="prevCredits") s.previousCredits = stoi(val);
        else if (key=="gradingMode")  s.gradingMode    = val;
        else if (key=="teacherAvg")   s.teacherAvg     = stod(val);
        else if (key.substr(0,10)=="relThresh_") s.relativeThresholds[key.substr(10)] = stod(val);
        else if (key.substr(0,9)=="boundary_") {
            string grade = key.substr(9);
            s.gradeBoundaries[grade] = stod(val);
        }
    }
    return true;
}

// ── Courses ───────────────────────────────────────────────────────────────────
bool FileManager::saveCourses(const vector<Course>& courses) {
    ofstream f(semesterPath());
    if (!f.is_open()) return false;
    f << "count=" << courses.size() << "\n";
    for (int i = 0; i < (int)courses.size(); i++) {
        const Course& c = courses[i];
        string p = "c" + to_string(i) + "_";
        f << p << "name=" << c.name << "\n";
        f << p << "code=" << c.shortCode << "\n";
        f << p << "theoryCredits=" << c.theoryCredits << "\n";
        f << p << "labCredits=" << c.labCredits << "\n";
        f << p << "hasLab=" << c.hasLab << "\n";
        f << p << "hasProject=" << c.hasProject << "\n";
        f << p << "quizW=" << c.quizW << "\n";
        f << p << "assignW=" << c.assignmentW << "\n";
        f << p << "midW=" << c.midW << "\n";
        f << p << "finalW=" << c.finalW << "\n";
        f << p << "projectW=" << c.projectW << "\n";
        f << p << "practicalW=" << c.practicalW << "\n";
        f << p << "labMidW=" << c.labMidW << "\n";
        f << p << "labFinalW=" << c.labFinalW << "\n";
        f << p << "labProjectW=" << c.labProjectW << "\n";
        f << p << "labHasProject=" << c.labHasProject << "\n";
        f << p << "quizCount=" << c.quizCount << "\n";
        f << p << "assignCount=" << c.assignmentCount << "\n";
        f << p << "practCount=" << c.practicalCount << "\n";
        f << p << "dropQuiz=" << c.dropLowestQuiz << "\n";
        f << p << "dropPract=" << c.dropLowestPractical << "\n";
        f << p << "targetGrade=" << c.targetGrade << "\n";
        f << p << "attended=" << c.attended << "\n";
        f << p << "totalClasses=" << c.totalClasses << "\n";
    }
    return true;
}

bool FileManager::loadCourses(vector<Course>& courses) {
    ifstream f(semesterPath());
    if (!f.is_open()) return false;
    string line;
    int count = 0;
    // First pass: get count
    while (getline(f, line)) {
        if (line.substr(0,6)=="count=") {
            count = stoi(line.substr(6));
            break;
        }
    }
    f.seekg(0);
    courses.clear();
    courses.resize(count);
    while (getline(f, line)) {
        if (line.empty() || line[0]=='#') continue;
        auto eq = line.find('=');
        if (eq == string::npos) continue;
        string key = line.substr(0, eq);
        string val = line.substr(eq + 1);
        if (key=="count") continue;
        // Parse prefix c{i}_field
        if (key.size()<2 || key[0]!='c') continue;
        int us = key.find('_');
        if (us == (int)string::npos) continue;
        int idx = stoi(key.substr(1, us-1));
        string field = key.substr(us+1);
        if (idx < 0 || idx >= count) continue;
        Course& c = courses[idx];
        if (field=="name")         c.name           = val;
        else if (field=="code")    c.shortCode      = val;
        else if (field=="theoryCredits") c.theoryCredits = stoi(val);
        else if (field=="labCredits")    c.labCredits    = stoi(val);
        else if (field=="hasLab")        c.hasLab        = stoi(val);
        else if (field=="hasProject")    c.hasProject    = stoi(val);
        else if (field=="quizW")         c.quizW         = stod(val);
        else if (field=="assignW")       c.assignmentW   = stod(val);
        else if (field=="midW")          c.midW          = stod(val);
        else if (field=="finalW")        c.finalW        = stod(val);
        else if (field=="projectW")      c.projectW      = stod(val);
        else if (field=="practicalW")    c.practicalW    = stod(val);
        else if (field=="labMidW")       c.labMidW       = stod(val);
        else if (field=="labFinalW")     c.labFinalW     = stod(val);
        else if (field=="labProjectW")   c.labProjectW   = stod(val);
        else if (field=="labHasProject") c.labHasProject = stoi(val);
        else if (field=="quizCount")     c.quizCount     = stoi(val);
        else if (field=="assignCount")   c.assignmentCount= stoi(val);
        else if (field=="practCount")    c.practicalCount = stoi(val);
        else if (field=="dropQuiz")      c.dropLowestQuiz = stoi(val);
        else if (field=="dropPract")     c.dropLowestPractical = stoi(val);
        else if (field=="targetGrade")   c.targetGrade   = val;
        else if (field=="attended")      c.attended      = stoi(val);
        else if (field=="totalClasses")  c.totalClasses  = stoi(val);
    }
    return true;
}

// ── Marks ─────────────────────────────────────────────────────────────────────
static void writeMarks(ofstream& f, const vector<Assessment>& marks,
                       const string& courseName, int sem, const string& session) {
    f << "# Course: " << courseName << " | Semester: " << sem
      << " | Session: " << session << "\n";
    f << "type,number,obtained,total,weightage,absolute,"
      << "mean,median,stddev,variance,topper,lowest,rank,size,percentile,abovemean,hasStats\n";
    for (auto& a : marks) {
        f << a.type << "," << a.number << ","
          << a.obtained << "," << a.total << ","
          << a.weightage << "," << a.absolute << ","
          << a.classMean << "," << a.classMedian << ","
          << a.classStdDev << "," << a.classVariance << ","
          << a.classTopper << "," << a.classLowest << ","
          << a.studentRank << "," << a.classSize << ","
          << a.studentPercentile << "," << a.marksAboveMean << ","
          << a.hasClassStats << "\n";
    }
}

bool FileManager::saveMarks(const Course& c) {
    // Theory
    {
        ofstream f(theoryPath(c.shortCode));
        if (!f.is_open()) return false;
        writeMarks(f, c.theoryMarks, c.name, 0, "");
    }
    // Lab
    if (c.hasLab) {
        ofstream f(labPath(c.shortCode));
        if (!f.is_open()) return false;
        writeMarks(f, c.labMarks, c.name, 0, "");
    }
    return true;
}

static void readMarks(ifstream& f, vector<Assessment>& marks) {
    string line;
    bool header = true;
    while (getline(f, line)) {
        if (line.empty() || line[0]=='#') continue;
        if (header) { header = false; continue; } // skip column header
        stringstream ss(line);
        string tok;
        Assessment a;
        int col = 0;
        while (getline(ss, tok, ',')) {
            switch(col++) {
                case 0:  a.type             = tok; break;
                case 1:  a.number           = stoi(tok); break;
                case 2:  a.obtained         = stod(tok); break;
                case 3:  a.total            = stod(tok); break;
                case 4:  a.weightage        = stod(tok); break;
                case 5:  a.absolute         = stod(tok); break;
                case 6:  a.classMean        = stod(tok); break;
                case 7:  a.classMedian      = stod(tok); break;
                case 8:  a.classStdDev      = stod(tok); break;
                case 9:  a.classVariance    = stod(tok); break;
                case 10: a.classTopper      = stod(tok); break;
                case 11: a.classLowest      = stod(tok); break;
                case 12: a.studentRank      = stoi(tok); break;
                case 13: a.classSize        = stoi(tok); break;
                case 14: a.studentPercentile= stod(tok); break;
                case 15: a.marksAboveMean   = stod(tok); break;
                case 16: a.hasClassStats    = stoi(tok); break;
            }
        }
        if (!a.type.empty()) marks.push_back(a);
    }
}

bool FileManager::loadMarks(Course& c) {
    c.theoryMarks.clear();
    c.labMarks.clear();
    {
        ifstream f(theoryPath(c.shortCode));
        if (f.is_open()) readMarks(f, c.theoryMarks);
    }
    if (c.hasLab) {
        ifstream f(labPath(c.shortCode));
        if (f.is_open()) readMarks(f, c.labMarks);
    }
    return true;
}

// ── Buffer file ───────────────────────────────────────────────────────────────
vector<pair<string,double>> FileManager::readBufferFile(string& errorMsg) {
    vector<pair<string,double>> rows;
    ifstream f(bufferPath());
    if (!f.is_open()) {
        errorMsg = "Cannot open " + bufferPath() + ". Create it and add class data.";
        return rows;
    }
    string line;
    int lineNum = 0;
    map<string,int> seenRegnos;
    while (getline(f, line)) {
        lineNum++;
        if (line.empty() || line[0]=='#') continue;
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n")+1);
        if (line.empty()) continue;

        auto comma = line.find(',');
        if (comma == string::npos) {
            errorMsg = "Line " + to_string(lineNum) + ": missing comma separator.";
            return {};
        }
        string regno = line.substr(0, comma);
        string markStr = line.substr(comma+1);
        regno.erase(0, regno.find_first_not_of(" \t"));
        regno.erase(regno.find_last_not_of(" \t")+1);
        markStr.erase(0, markStr.find_first_not_of(" \t"));
        markStr.erase(markStr.find_last_not_of(" \t")+1);

        // Check duplicate regno
        if (seenRegnos.count(regno)) {
            errorMsg = "Duplicate regno '" + regno + "' on lines "
                     + to_string(seenRegnos[regno]) + " and " + to_string(lineNum) + ".";
            return {};
        }
        seenRegnos[regno] = lineNum;

        // Validate mark is numeric
        try {
            double mark = stod(markStr);
            if (mark < 0) {
                errorMsg = "Line " + to_string(lineNum) + ": negative marks not allowed.";
                return {};
            }
            rows.push_back({regno, mark});
        } catch (...) {
            // Non-numeric — skip with warning, don't reject all
            cerr << "  [Warning] Line " << lineNum << ": non-numeric mark '"
                 << markStr << "' skipped.\n";
        }
    }
    return rows;
}

void FileManager::wipeBufferFile() {
    ofstream f(bufferPath(), ios::trunc);
    f << "# Format: regno,marks\n";
}

// ── History ───────────────────────────────────────────────────────────────────
bool FileManager::appendHistory(const SemesterRecord& rec) {
    ofstream f(historyPath(), ios::app);
    if (!f.is_open()) return false;
    f << "---\n";
    f << "session=" << rec.sessionLabel << "\n";
    f << "semester=" << rec.semesterNumber << "\n";
    f << "sgpa=" << rec.sgpa << "\n";
    f << "credits=" << rec.credits << "\n";
    return true;
}

bool FileManager::loadHistory(vector<SemesterRecord>& history) {
    ifstream f(historyPath());
    if (!f.is_open()) return false;
    history.clear();
    string line;
    SemesterRecord cur;
    bool inRecord = false;
    while (getline(f, line)) {
        if (line == "---") {
            if (inRecord) history.push_back(cur);
            cur = SemesterRecord();
            inRecord = true;
            continue;
        }
        auto eq = line.find('=');
        if (eq == string::npos) continue;
        string key = line.substr(0, eq);
        string val = line.substr(eq+1);
        if (key=="session")  cur.sessionLabel    = val;
        else if (key=="semester") cur.semesterNumber = stoi(val);
        else if (key=="sgpa")     cur.sgpa           = stod(val);
        else if (key=="credits")  cur.credits        = stoi(val);
    }
    if (inRecord) history.push_back(cur);
    return true;
}

// ── Report ────────────────────────────────────────────────────────────────────
bool FileManager::generateReport(const Student& s, const vector<Course>& courses) {
    string filename = "reports/" + sanitize(s.sessionLabel)
                    + "_Report_" + todayString() + ".txt";
    // Avoid overwrite
    int counter = 1;
    while (fs::exists(filename)) {
        filename = "reports/" + sanitize(s.sessionLabel)
                 + "_Report_" + todayString() + "_" + to_string(++counter) + ".txt";
    }
    ofstream f(filename);
    if (!f.is_open()) return false;

    f << "============================================================\n";
    f << "  GIKI STUDENT STUDY MANAGER — SEMESTER REPORT\n";
    f << "============================================================\n";
    f << "Student : " << s.name << "\n";
    f << "Regno   : " << s.regno << "\n";
    f << "Semester: " << s.currentSemester << " | Session: " << s.sessionLabel << "\n";
    f << "------------------------------------------------------------\n\n";

    f << "SECTION 1 — COURSES\n";
    for (auto& c : courses) {
        f << "\n  " << c.name << " (" << c.shortCode << ")\n";
        f << "  Theory Credits: " << c.theoryCredits;
        if (c.hasLab) f << " | Lab Credits: " << c.labCredits;
        f << "\n";
        f << "  Target Grade: " << c.targetGrade << "\n";
        f << "  Attendance: " << c.attended << "/" << c.totalClasses;
        if (c.totalClasses > 0)
            f << " (" << (100.0*c.attended/c.totalClasses) << "%)";
        f << "\n";
        f << "  Theory Marks:\n";
        for (auto& a : c.theoryMarks)
            f << "    " << a.type << " " << a.number << ": "
              << a.obtained << "/" << a.total
              << "  abs=" << a.absolute << "%\n";
        if (c.hasLab) {
            f << "  Lab Marks:\n";
            for (auto& a : c.labMarks)
                f << "    " << a.type << " " << a.number << ": "
                  << a.obtained << "/" << a.total
                  << "  abs=" << a.absolute << "%\n";
        }
    }

    f << "\n------------------------------------------------------------\n";
    f << "SECTION 2 — SEMESTER HISTORY\n";
    for (auto& r : s.history)
        f << "  " << r.sessionLabel << " (Sem " << r.semesterNumber
          << "): SGPA = " << r.sgpa << "\n";

    f << "\n============================================================\n";
    f << "  End of Report\n";
    f << "============================================================\n";

    cout << "  Report saved: " << filename << "\n";
    return true;
}
