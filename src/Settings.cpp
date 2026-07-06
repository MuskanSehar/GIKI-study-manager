#include "Settings.h"
#include "Dashboard.h"
#include <iostream>
#include <iomanip>
#include <limits>
using namespace std;

static double getDouble(const string& prompt, double lo=0, double hi=1e9) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= lo && v <= hi) return v;
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n');
        cout << "  Invalid. Enter a number between " << lo << " and " << hi << ".\n";
    }
}
static int getInt(const string& prompt, int lo=0, int hi=100) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= lo && v <= hi) return v;
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n');
        cout << "  Invalid. Enter a number between " << lo << " and " << hi << ".\n";
    }
}
static bool confirm(const string& msg) {
    string r;
    cout << msg << " (yes/no): ";
    cin >> r;
    return (r=="yes" || r=="y" || r=="YES");
}

// ── Main settings menu ────────────────────────────────────────────────────────
void Settings::showMenu(Student& s, vector<Course>& courses) {
    while (true) {
        Dashboard::clearScreen();
        Dashboard::printHeader("Settings");
        cout << "\n  [1] Course Settings\n";
        cout << "  [2] Profile Settings\n";
        cout << "  [3] Grade Settings\n";
        cout << "  [4] Semester Management\n";
        cout << "  [0] Back\n";
        Dashboard::printLine('=');
        cout << "  Choice: ";
        string ch; cin >> ch;
        if (ch=="1") courseSettings(s, courses);
        else if (ch=="2") profileSettings(s, courses);
        else if (ch=="3") gradeSettings(s, courses);
        else if (ch=="4") semesterManagement(s, courses);
        else if (ch=="0") break;
    }
}

// ── Course Settings ───────────────────────────────────────────────────────────
void Settings::courseSettings(Student& s, vector<Course>& courses) {
    Dashboard::clearScreen();
    Dashboard::printHeader("Course Settings");
    for (int i = 0; i < (int)courses.size(); i++)
        cout << "  [" << i+1 << "] " << courses[i].name << "\n";
    cout << "  Select course (0=back): ";
    int idx; cin >> idx;
    if (idx <= 0 || idx > (int)courses.size()) return;
    idx--;
    Course& c = courses[idx];

    cout << "\n  [1] Edit weightages\n";
    cout << "  [2] Toggle drop-lowest quiz\n";
    cout << "  [3] Toggle drop-lowest practical\n";
    cout << "  [4] Add extra assessment count\n";
    cout << "  [5] Update attendance\n";
    cout << "  [6] Remove this course\n";
    cout << "  [7] Set target grade\n";
    cout << "  Choice: ";
    string ch; cin >> ch;

    if (ch=="1") editWeightage(c, s, courses);
    else if (ch=="2") toggleDropLowest(c, courses);
    else if (ch=="3") {
        c.dropLowestPractical = !c.dropLowestPractical;
        cout << "  Drop-lowest practical: " << (c.dropLowestPractical?"ON":"OFF") << "\n";
        FileManager::saveCourses(courses);
    }
    else if (ch=="4") addAssessment(c, courses);
    else if (ch=="5") updateAttendance(c, courses);
    else if (ch=="6") removeCourse(courses, idx);
    else if (ch=="7") {
        cout << "  Target grade (e.g. B+): "; cin >> c.targetGrade;
        FileManager::saveCourses(courses);
        cout << Dashboard::GREEN << "  Target grade updated.\n" << Dashboard::RESET;
    }
    Dashboard::pause();
}

void Settings::editWeightage(Course& c, Student& s, vector<Course>& courses) {
    bool hasMarks = !c.theoryMarks.empty();
    if (hasMarks) {
        cout << Dashboard::YELLOW
             << "  Warning: marks already exist. Weightages will be recalculated.\n"
             << Dashboard::RESET;
        if (!confirm("  Continue?")) return;
    }
    cout << "  Enter new weightages (must sum to 100)\n";
    double remaining = 100;
    if (c.quizCount > 0) {
        c.quizW = getDouble("  Quiz category %: ", 0, remaining);
        remaining -= c.quizW;
    }
    if (c.assignmentCount > 0) {
        c.assignmentW = getDouble("  Assignment category %: ", 0, remaining);
        remaining -= c.assignmentW;
    }
    c.midW = getDouble("  Mid %: ", 0, remaining);
    remaining -= c.midW;
    if (c.hasProject) {
        c.projectW = getDouble("  Project %: ", 0, remaining);
        remaining -= c.projectW;
    }
    c.finalW = remaining;
    cout << "  Final automatically set to: " << c.finalW << "%\n";

    // Recalculate existing marks
    if (hasMarks) {
        GradeEngine::redistributeWeightage(c.theoryMarks, "Quiz", c.quizW);
        GradeEngine::redistributeWeightage(c.theoryMarks, "Assignment", c.assignmentW);
        for (auto& a : c.theoryMarks) {
            if (a.type=="Mid")     { a.weightage=c.midW;     a.absolute=GradeEngine::calcAbsolute(a.obtained,a.total,a.weightage); }
            if (a.type=="Final")   { a.weightage=c.finalW;   a.absolute=GradeEngine::calcAbsolute(a.obtained,a.total,a.weightage); }
            if (a.type=="Project") { a.weightage=c.projectW; a.absolute=GradeEngine::calcAbsolute(a.obtained,a.total,a.weightage); }
        }
        FileManager::saveMarks(c);
    }
    FileManager::saveCourses(courses);
    cout << Dashboard::GREEN << "  Weightages updated.\n" << Dashboard::RESET;
}

void Settings::addAssessment(Course& c, vector<Course>& courses) {
    cout << "  [1] Add quiz  [2] Add assignment  [3] Add practical: ";
    string ch; cin >> ch;
    if (ch=="1") {
        c.quizCount++;
        GradeEngine::redistributeWeightage(c.theoryMarks, "Quiz", c.quizW);
    } else if (ch=="2") {
        c.assignmentCount++;
        GradeEngine::redistributeWeightage(c.theoryMarks, "Assignment", c.assignmentW);
    } else if (ch=="3") {
        c.practicalCount++;
        GradeEngine::redistributeWeightage(c.labMarks, "Practical", c.practicalW);
    }
    FileManager::saveCourses(courses);
    cout << Dashboard::GREEN << "  Assessment count updated.\n" << Dashboard::RESET;
}

void Settings::removeCourse(vector<Course>& courses, int idx) {
    Course& c = courses[idx];
    if (!c.theoryMarks.empty() || !c.labMarks.empty()) {
        cout << Dashboard::RED
             << "  WARNING: " << c.name << " has "
             << c.theoryMarks.size() + c.labMarks.size()
             << " mark entries that will be permanently deleted.\n"
             << Dashboard::RESET;
        cout << "  Type DELETE to confirm: ";
        string input; cin >> input;
        if (input != "DELETE") { cout << "  Cancelled.\n"; return; }
    }
    courses.erase(courses.begin() + idx);
    FileManager::saveCourses(courses);
    cout << Dashboard::GREEN << "  Course removed.\n" << Dashboard::RESET;
}

void Settings::updateAttendance(Course& c, vector<Course>& courses) {
    c.attended     = getInt("  Classes attended: ", 0, 500);
    c.totalClasses = getInt("  Total classes held: ", 0, 500);
    FileManager::saveCourses(courses);
    if (c.totalClasses > 0) {
        double pct = 100.0 * c.attended / c.totalClasses;
        if (pct < 75)
            cout << Dashboard::RED << "  Warning: below 75% attendance (" << pct << "%).\n" << Dashboard::RESET;
        else
            cout << Dashboard::GREEN << "  Attendance: " << pct << "%\n" << Dashboard::RESET;
    }
}

void Settings::toggleDropLowest(Course& c, vector<Course>& courses) {
    c.dropLowestQuiz = !c.dropLowestQuiz;
    cout << "  Drop-lowest quiz: " << (c.dropLowestQuiz?"ON":"OFF") << "\n";
    FileManager::saveCourses(courses);
}

// ── Profile Settings ──────────────────────────────────────────────────────────
void Settings::profileSettings(Student& s, vector<Course>& courses) {
    Dashboard::clearScreen();
    Dashboard::printHeader("Profile Settings");
    cout << "\n  [1] Change name\n  [2] Change session label\n";
    cout << "  [3] Change regno\n  Choice: ";
    string ch; cin >> ch;
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    if (ch=="1") {
        cout << "  New name: "; getline(cin, s.name);
    } else if (ch=="2") {
        cout << "  New session label (e.g. Fall 2026): "; getline(cin, s.sessionLabel);
    } else if (ch=="3") {
        cout << Dashboard::YELLOW
             << "  Warning: changing regno only affects future buffer file reads.\n"
             << "  Past marks are unaffected.\n" << Dashboard::RESET;
        if (confirm("  Continue?")) {
            cout << "  New regno: "; cin >> s.regno;
        }
    }
    FileManager::saveProfile(s);
    cout << Dashboard::GREEN << "  Profile updated.\n" << Dashboard::RESET;
    Dashboard::pause();
}

// ── Grade Settings ────────────────────────────────────────────────────────────
void Settings::gradeSettings(Student& s, vector<Course>& courses) {
    Dashboard::clearScreen();
    Dashboard::printHeader("Grade Settings");
    cout << "\n  Current grade boundaries:\n";
    const vector<string> order = {"A+","A","A-","B+","B","B-","C+","C","C-","D","F"};
    for (auto& g : order) {
        auto it = s.gradeBoundaries.find(g);
        if (it != s.gradeBoundaries.end())
            cout << "  " << setw(4) << g << ": " << it->second << "%\n";
    }
    cout << "\n";
    if (!confirm("  Edit boundaries?")) { Dashboard::pause(); return; }
    cout << Dashboard::YELLOW
         << "  Warning: all grade predictions will recalculate immediately.\n"
         << Dashboard::RESET;
    if (!confirm("  Continue?")) { Dashboard::pause(); return; }

    double prev = 100;
    for (auto& g : {"A+","A","A-","B+","B","B-","C+","C","C-","D"}) {
        double v = getDouble(string("  ")+g+" cutoff % (max "+to_string((int)prev)+"): ", 0, prev);
        s.gradeBoundaries[g] = v;
        prev = v;
    }
    FileManager::saveProfile(s);
    cout << Dashboard::GREEN << "  Grade boundaries updated.\n" << Dashboard::RESET;
    Dashboard::pause();
}

// ── Semester Management ───────────────────────────────────────────────────────
void Settings::semesterManagement(Student& s, vector<Course>& courses) {
    Dashboard::clearScreen();
    Dashboard::printHeader("Semester Management — End Semester");

    // Check for empty courses
    int empty = 0;
    for (auto& c : courses)
        if (c.theoryMarks.empty()) empty++;
    if (empty > 0) {
        cout << Dashboard::YELLOW
             << "\n  Warning: " << empty << " course(s) have no marks entered.\n"
             << Dashboard::RESET;
        if (!confirm("  Continue with End Semester?")) { Dashboard::pause(); return; }
    } else {
        if (!confirm("\n  End this semester and archive all data?")) {
            Dashboard::pause(); return;
        }
    }

    // Calculate final SGPA
    double finalSGPA = GradeEngine::calcSemesterSGPA(courses, s);
    int semCredits = 0;
    for (auto& c : courses) semCredits += c.theoryCredits + c.labCredits;

    // Generate report
    cout << "\n  Generating report...\n";
    FileManager::generateReport(s, courses);

    // Archive to history
    SemesterRecord rec;
    rec.sessionLabel    = s.sessionLabel;
    rec.semesterNumber  = s.currentSemester;
    rec.sgpa            = finalSGPA;
    rec.credits         = semCredits;
    s.history.push_back(rec);
    FileManager::appendHistory(rec);

    // Update CGPA for next semester
    double newCGPA     = GradeEngine::calcProjectedCGPA(s, finalSGPA, semCredits);
    s.previousCGPA    = newCGPA;
    s.previousCredits += semCredits;
    s.currentSemester++;

    cout << "  Semester archived. Final SGPA: "
         << Dashboard::CYAN << fixed << setprecision(2) << finalSGPA << Dashboard::RESET
         << "  |  New CGPA: "
         << Dashboard::GREEN << fixed << setprecision(2) << newCGPA << Dashboard::RESET << "\n";

    // Clear current semester data
    courses.clear();
    cout << "\n  Starting setup for new semester...\n";
    Dashboard::pause();
    // Caller (main) will detect empty courses and re-run setup
}
