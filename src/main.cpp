#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include "Student.h"
#include "Course.h"
#include "Assessment.h"
#include "GradeEngine.h"
#include "FileManager.h"
#include "Dashboard.h"
#include "Settings.h"
using namespace std;

// ── Input helpers ─────────────────────────────────────────────────────────────
static double getDouble(const string& p, double lo=0, double hi=1e9) {
    double v;
    while (true) {
        cout << p;
        if (cin >> v && v >= lo && v <= hi) return v;
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n');
        cout << "  Invalid. Enter a number between " << lo << " and " << hi << ".\n";
    }
}
static int getInt(const string& p, int lo=0, int hi=1000) {
    int v;
    while (true) {
        cout << p;
        if (cin >> v && v >= lo && v <= hi) return v;
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n');
        cout << "  Invalid. Enter a number between " << lo << " and " << hi << ".\n";
    }
}
static string getString(const string& p) {
    string v; cout << p; cin >> v; return v;
}

// ── Demo mode setup ───────────────────────────────────────────────────────────
// Hidden feature: type "demo" at the main menu to load a pre-built semester
void loadDemo(Student& s, vector<Course>& courses) {
    // ── Student profile ────────────────────────────────────────────
    s.regno           = "2025049";
    s.name            = "Abdullah Sultan";
    s.sessionLabel    = "Spring 2026";
    s.currentSemester = 1;   // first semester — no previous CGPA
    s.previousCGPA    = 0.0;
    s.previousCredits = 0;
    s.gradingMode     = "relative";
    s.teacherAvg      = 72.0;
    s.relativeThresholds = {{"A",1.5},{"B",0.5},{"C",-0.5},{"D",-1.5}};

    courses.clear();

    // Helper lambda: build a 45-student class vector with given mean and SD
    // using a linear spread: mean + (i-22)*(sd/11.0)
    auto makeClass = [](double mean, double sd, double maxMark) -> vector<double> {
        vector<double> v;
        v.reserve(45);
        for (int i = 0; i < 45; ++i) {
            double val = mean + (i - 22) * (sd / 11.0);
            if (val < 0)       val = 0;
            if (val > maxMark) val = maxMark;
            v.push_back(val);
        }
        return v;
    };

    // ── Course 1: MT102 — Mathematics ─────────────────────────────
    {
        Course c;
        c.name="Mathematics"; c.shortCode="MT102";
        c.theoryCredits=3; c.labCredits=0; c.hasLab=false;
        c.quizCount=4; c.assignmentCount=4;
        c.quizW=20; c.assignmentW=10; c.midW=30; c.finalW=40;
        c.targetGrade="A";

        double qw = c.quizW / c.quizCount;           // 5 per quiz
        double aw = c.assignmentW / c.assignmentCount; // 2.5 per asg

        // 4 quizzes: strong student ~8-9/10, class mean ~6.7/10, sd ~1.35/10
        vector<double> qObt = {8,9,8,9};
        for (int i = 0; i < 4; ++i) {
            Assessment a("Quiz", i+1, qObt[i], 10, qw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(6.7, 1.35, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // 4 assignments: 17-19/20, class mean ~13.5/20, sd ~2.5/20
        vector<double> aObt = {17,19,18,17};
        for (int i = 0; i < 4; ++i) {
            Assessment a("Assignment", i+1, aObt[i], 20, aw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(13.5, 2.5, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Mid: 40/50, class mean ~33.5/50, sd ~6.0/50
        {
            Assessment a("Mid", 1, 40, 50, c.midW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(33.5, 6.0, 50);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        c.upcomingDates.push_back({"Final Exam","2026-06-12"});
        courses.push_back(c);
    }

    // ── Course 2: ES112 — Engineering Sciences ─────────────────────
    {
        Course c;
        c.name="Engineering Sciences"; c.shortCode="ES112";
        c.theoryCredits=3; c.labCredits=0; c.hasLab=false;
        c.quizCount=5; c.assignmentCount=5;
        c.hasProject=true;
        c.quizW=20; c.assignmentW=6; c.projectW=4; c.midW=30; c.finalW=40;
        c.targetGrade="A-";

        double qw = c.quizW / c.quizCount;             // 4 per quiz
        double aw = c.assignmentW / c.assignmentCount;  // 1.2 per asg

        // 5 quizzes: 8-9/10, class mean ~6.15/10, sd ~1.2/10
        vector<double> qObt = {8,9,8,8,9};
        for (int i = 0; i < 5; ++i) {
            Assessment a("Quiz", i+1, qObt[i], 10, qw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(6.15, 1.2, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // 5 assignments: 8-9/10, class mean ~6.15/10, sd ~1.2/10
        vector<double> aObt = {8,9,8,9,8};
        for (int i = 0; i < 5; ++i) {
            Assessment a("Assignment", i+1, aObt[i], 10, aw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(6.15, 1.2, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Project: 18/20, class mean ~12.5/20, sd ~2.4/20
        {
            Assessment a("Project", 1, 18, 20, c.projectW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(12.5, 2.4, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Mid: 42/50, class mean ~31.25/50, sd ~6.0/50
        {
            Assessment a("Mid", 1, 42, 50, c.midW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(31.25, 6.0, 50);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        c.upcomingDates.push_back({"Final Exam","2026-06-14"});
        courses.push_back(c);
    }

    // ── Course 3: CS102 — Computer Science (theory+lab) ────────────
    {
        Course c;
        c.name="Computer Science"; c.shortCode="CS102";
        c.theoryCredits=2; c.labCredits=1; c.hasLab=true;
        c.quizCount=4; c.assignmentCount=4;
        c.quizW=20; c.assignmentW=10; c.midW=30; c.finalW=40;
        // Lab: Lab Task 25%, Mid 20%, Final 35%, Project 20%
        c.practicalCount=12;
        c.practicalW=25; c.labMidW=20; c.labFinalW=35; c.labProjectW=20;
        c.labHasProject=true;
        c.targetGrade="A";

        double qw = c.quizW / c.quizCount;
        double aw = c.assignmentW / c.assignmentCount;

        // 4 quizzes: 9/10, class mean ~6.8/10, sd ~1.1/10
        for (int i = 0; i < 4; ++i) {
            Assessment a("Quiz", i+1, 9, 10, qw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(6.8, 1.1, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // 4 assignments: 18-20/20, class mean ~13.6/20, sd ~2.2/20
        vector<double> aObt = {18,20,19,18};
        for (int i = 0; i < 4; ++i) {
            Assessment a("Assignment", i+1, aObt[i], 20, aw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(13.6, 2.2, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Mid: 44/50, class mean ~34/50, sd ~5.5/50
        {
            Assessment a("Mid", 1, 44, 50, c.midW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(34.0, 5.5, 50);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Lab: 12 lab tasks (9-10/10), class mean ~7.5/10, sd ~0.8/10
        double ltW = c.practicalW / c.practicalCount;
        vector<double> ltObt = {9,10,9,10,9,10,9,9,10,9,10,9};
        for (int i = 0; i < 12; ++i) {
            Assessment a("Practical", i+1, ltObt[i], 10, ltW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(7.5, 0.8, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        // Lab Mid: 19/20, class mean ~15/20, sd ~1.6/20
        {
            Assessment a("Lab Mid", 1, 19, 20, c.labMidW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(15.0, 1.6, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        // Lab Project: 17/20, class mean ~15/20, sd ~1.6/20
        {
            Assessment a("Project", 1, 17, 20, c.labProjectW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(15.0, 1.6, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        c.upcomingDates.push_back({"Final Exam","2026-06-10"});
        c.upcomingDates.push_back({"Lab Final","2026-06-08"});
        courses.push_back(c);
    }

    // ── Course 4: MM102 — Mathematics II (theory+lab) ──────────────
    {
        Course c;
        c.name="Mathematics II"; c.shortCode="MM102";
        c.theoryCredits=2; c.labCredits=1; c.hasLab=true;
        c.quizCount=4; c.assignmentCount=4;
        c.quizW=20; c.assignmentW=10; c.midW=30; c.finalW=40;
        // Lab: Practical 50%, Mid 20%, Final 30%
        c.practicalCount=12;
        c.practicalW=50; c.labMidW=20; c.labFinalW=30;
        c.targetGrade="B+";

        double qw = c.quizW / c.quizCount;
        double aw = c.assignmentW / c.assignmentCount;

        // 4 quizzes: 8-9/10, class mean ~6.3/10, sd ~1.3/10
        vector<double> qObt = {8,9,8,8};
        for (int i = 0; i < 4; ++i) {
            Assessment a("Quiz", i+1, qObt[i], 10, qw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(6.3, 1.3, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // 4 assignments: 16-19/20, class mean ~12.6/20, sd ~2.6/20
        vector<double> aObt = {16,19,17,18};
        for (int i = 0; i < 4; ++i) {
            Assessment a("Assignment", i+1, aObt[i], 20, aw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(12.6, 2.6, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Mid: 38/50, class mean ~31.5/50, sd ~6.5/50
        {
            Assessment a("Mid", 1, 38, 50, c.midW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(31.5, 6.5, 50);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Lab: 12 practicals (8-10/10), class mean ~7.0/10, sd ~0.9/10
        double pW = c.practicalW / c.practicalCount;
        vector<double> pObt = {8,10,9,8,10,9,8,9,10,8,9,8};
        for (int i = 0; i < 12; ++i) {
            Assessment a("Practical", i+1, pObt[i], 10, pW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(7.0, 0.9, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        // Lab Mid: 17/20, class mean ~14.0/20, sd ~1.8/20
        {
            Assessment a("Lab Mid", 1, 17, 20, c.labMidW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(14.0, 1.8, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        c.upcomingDates.push_back({"Final Exam","2026-06-16"});
        c.upcomingDates.push_back({"Lab Final","2026-06-09"});
        courses.push_back(c);
    }

    // ── Course 5: IF102 — Islamiat/Pakistan Studies (lab-only) ─────
    {
        Course c;
        c.name="Islamiat/Pakistan Studies"; c.shortCode="IF102";
        c.theoryCredits=0; c.labCredits=1; c.hasLab=true;
        // Lab only: Practical 100%
        c.practicalCount=12; c.practicalW=100;
        c.targetGrade="A+";

        double pW = c.practicalW / c.practicalCount;

        // 12 lab tasks: 9-10/10, class mean ~8.0/10, sd ~0.6/10
        vector<double> pObt = {9,10,9,10,10,9,10,9,10,9,10,9};
        for (int i = 0; i < 12; ++i) {
            Assessment a("Practical", i+1, pObt[i], 10, pW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(8.0, 0.6, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        c.upcomingDates.push_back({"Last Lab","2026-06-03"});
        courses.push_back(c);
    }

    // ── Course 6: HM102 — Humanities (theory+lab) ──────────────────
    {
        Course c;
        c.name="Humanities"; c.shortCode="HM102";
        c.theoryCredits=2; c.labCredits=1; c.hasLab=true;
        c.quizCount=3; c.assignmentCount=3;
        c.quizW=20; c.assignmentW=10; c.midW=30; c.finalW=40;
        // Lab: Assignment 30%, Mid Task 30%, Final Task 40%
        c.assignmentCount=3; // theory assignments
        // Lab uses practicalW=0, custom mapping via labMidW, labFinalW, labProjectW
        // We'll store lab assignments as "Assignment" type in labMarks
        // Lab: 4 assignments (30% total), 1 lab mid (30%), lab final missing (40%)
        c.practicalCount=0;
        c.practicalW=0;
        c.labMidW=30; c.labFinalW=40; c.labProjectW=30;
        c.labHasProject=true;
        c.targetGrade="A-";

        double qw = c.quizW / c.quizCount;
        double aw = c.assignmentW / c.assignmentCount;

        // 3 quizzes: 8-9/10, class mean ~6.6/10, sd ~1.2/10
        vector<double> qObt = {8,9,8};
        for (int i = 0; i < 3; ++i) {
            Assessment a("Quiz", i+1, qObt[i], 10, qw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(6.6, 1.2, 10);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // 3 assignments: 17-19/20, class mean ~13.2/20, sd ~2.4/20
        vector<double> aObt = {17,19,18};
        for (int i = 0; i < 3; ++i) {
            Assessment a("Assignment", i+1, aObt[i], 20, aw);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(13.2, 2.4, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Mid: 40/50, class mean ~33.0/50, sd ~6.0/50
        {
            Assessment a("Mid", 1, 40, 50, c.midW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(33.0, 6.0, 50);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.theoryMarks.push_back(a);
        }

        // Lab: 4 assignments (30% total = 7.5% each), class mean ~13.6/20, sd ~2.0/20
        double labAsgW = c.labProjectW / 4.0;
        vector<double> labAObt = {14,16,15,14};
        for (int i = 0; i < 4; ++i) {
            Assessment a("Assignment", i+1, labAObt[i], 20, labAsgW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(13.6, 2.0, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        // Lab Mid Task: 17/20, class mean ~13.6/20, sd ~2.0/20
        {
            Assessment a("Lab Mid", 1, 17, 20, c.labMidW);
            a.absolute = GradeEngine::calcAbsolute(a.obtained, a.total, a.weightage);
            auto cls = makeClass(13.6, 2.0, 20);
            a = GradeEngine::calcClassStats(cls, a.obtained, a);
            c.labMarks.push_back(a);
        }

        c.upcomingDates.push_back({"Final Exam","2026-06-18"});
        c.upcomingDates.push_back({"Lab Final Task","2026-06-11"});
        courses.push_back(c);
    }

    // Save everything to disk
    FileManager::saveProfile(s);
    FileManager::saveCourses(courses);
    for (auto& c : courses) FileManager::saveMarks(c);

    cout << Dashboard::GREEN << "\n  Demo environment loaded! (Abdullah Sultan — Spring 2026)\n" << Dashboard::RESET;
    Dashboard::pause();
}

// ── Course setup ──────────────────────────────────────────────────────────────
Course setupCourse(const vector<Course>& existing) {
    Course c;
    while (true) {
        cout << "  Course name: ";
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
        getline(cin, c.name);
        bool dup = false;
        for (auto& e : existing) if (e.name==c.name) { dup=true; break; }
        if (!dup) break;
        cout << "  Duplicate name.\n";
    }
    c.shortCode     = getString("  Short code (e.g. CALC): ");
    c.theoryCredits = getInt("  Theory credit hours (0 if lab-only): ", 0, 10);
    c.labCredits    = getInt("  Lab credit hours (0 if no lab): ", 0, 5);
    c.hasLab        = (c.labCredits > 0);

    if (!c.isLabOnly()) {
        cout << "\n  Theory weightages (must sum to 100)\n";
        c.quizCount      = getInt("  Number of quizzes: ", 0, 50);
        c.assignmentCount= getInt("  Number of assignments: ", 0, 50);
        string hp = getString("  Has project? (y/n): ");
        c.hasProject = (hp=="y"||hp=="Y");
        double rem = 100.0;
        cout << "  Remaining: " << rem << "%\n";
        if (c.quizCount > 0) {
            c.quizW = getDouble("  Quiz category %: ", 0, rem);
            rem -= c.quizW; cout << "  Remaining: " << rem << "%\n";
        }
        if (c.assignmentCount > 0) {
            c.assignmentW = getDouble("  Assignment category %: ", 0, rem);
            rem -= c.assignmentW; cout << "  Remaining: " << rem << "%\n";
        }
        c.midW = getDouble("  Mid %: ", 0, rem);
        rem -= c.midW; cout << "  Remaining: " << rem << "%\n";
        if (c.hasProject) {
            c.projectW = getDouble("  Project %: ", 0, rem);
            rem -= c.projectW; cout << "  Remaining: " << rem << "%\n";
        }
        c.finalW = rem;
        cout << "  Final set to: " << c.finalW << "%\n";
        if (c.quizCount > 0) {
            string dq = getString("  Drop lowest quiz? (y/n): ");
            c.dropLowestQuiz = (dq=="y"||dq=="Y");
        }
    }

    c.targetGrade = getString("  Target grade (e.g. B+): ");

    if (c.hasLab) {
        cout << "\n  Lab configuration\n";
        c.practicalCount = getInt("  Number of practicals: ", 0, 30);
        bool hasLabMid   = (getString("  Lab mid? (y/n): ")=="y");
        bool hasLabFinal = (getString("  Lab final? (y/n): ")=="y");
        c.labHasProject  = (getString("  Lab project? (y/n): ")=="y");
        double labRem = 100.0;
        if (c.practicalCount > 0) {
            c.practicalW = getDouble("  Practical %: ", 0, labRem);
            labRem -= c.practicalW;
        }
        if (hasLabMid)  { c.labMidW = getDouble("  Lab mid %: ", 0, labRem); labRem -= c.labMidW; }
        if (c.labHasProject) { c.labProjectW = getDouble("  Lab project %: ", 0, labRem); labRem -= c.labProjectW; }
        if (hasLabFinal) { c.labFinalW = labRem; cout << "  Lab final set to: " << c.labFinalW << "%\n"; }
        if (c.practicalCount > 0) {
            c.dropLowestPractical = (getString("  Drop lowest practical? (y/n): ")=="y");
        }
    }
    return c;
}

// ── First-time setup ──────────────────────────────────────────────────────────
void runSetup(Student& s, vector<Course>& courses) {
    Dashboard::clearScreen();
    Dashboard::printHeader("First-Time Setup");
    cout << "\n  Welcome! Let's set up your study manager.\n\n";

    cout << "  Your name    : "; cin.ignore(); getline(cin, s.name);
    cout << "  Your regno   : "; cin >> s.regno;
    s.currentSemester = getInt("  Semester no. (1-8): ", 1, 8);
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    cout << "  Session label (e.g. Spring 2026): "; getline(cin, s.sessionLabel);

    if (s.currentSemester > 1) {
        s.previousCGPA    = getDouble("  Previous CGPA (0.00-4.00): ", 0, 4.0);
        s.previousCredits = getInt("  Total credits earned so far: ", 1, 300);
    }

    // ── Grading mode (per semester) ───────────────────────────────
    cout << "\n  Grading mode for this semester:\n";
    cout << "  [1] Absolute (fixed % cutoffs — default for GIKI)\n";
    cout << "  [2] Relative (normal distribution — if your teacher curves)\n";
    string gm = getString("  Choice (1/2): ");
    if (gm == "2") {
        s.gradingMode = "relative";
        cout << "\n  Default SD thresholds: A=+1.5SD, B=+0.5SD, C=-0.5SD, D=-1.5SD\n";
        string cust = getString("  Customise thresholds? (y/n): ");
        if (cust=="y"||cust=="Y") {
            s.relativeThresholds["A"] = getDouble("  A threshold (SD above mean, e.g. 1.5): ", 0, 5);
            s.relativeThresholds["B"] = getDouble("  B threshold (e.g. 0.5): ", -3, s.relativeThresholds["A"]);
            s.relativeThresholds["C"] = getDouble("  C threshold (e.g. -0.5): ", -5, s.relativeThresholds["B"]);
            s.relativeThresholds["D"] = getDouble("  D threshold (e.g. -1.5): ", -5, s.relativeThresholds["C"]);
        }
        cout << "\n  Teacher average (overall class % that your teacher targets)\n";
        cout << "  This helps calibrate relative grading.\n";
        string ta = getString("  Do you know the teacher's target average? (y/n): ");
        if (ta=="y"||ta=="Y") {
            s.teacherAvg = getDouble("  Teacher average (%): ", 0, 100);
        } else {
            s.teacherAvg = -1;
            cout << "  " << Dashboard::YELLOW
                 << "Reminder: add teacher average in Settings > Grade Settings when known."
                 << Dashboard::RESET << "\n";
        }
    } else {
        s.gradingMode = "absolute";
        // Grade boundaries
        cout << "\n  Default: A+=90, A=85, A-=80, B+=75, B=70, B-=65, C+=60, C=55, C-=50, D=45\n";
        string custom = getString("  Customise grade boundaries? (y/n): ");
        if (custom=="y"||custom=="Y") {
            double prev = 100;
            for (auto& g : {"A+","A","A-","B+","B","B-","C+","C","C-","D"}) {
                double v = getDouble(string("  ")+g+" starts at %: ", 0, prev);
                s.gradeBoundaries[g] = v;
                prev = v;
            }
        }
    }

    // Courses
    int n = getInt("\n  How many courses this semester? (1-10): ", 1, 10);
    for (int i=0;i<n;i++) {
        cout << "\n  --- Course " << i+1 << " of " << n << " ---\n";
        courses.push_back(setupCourse(courses));
    }

    // Credit warning
    int totalC = GradeEngine::totalSemCredits(courses);
    if (totalC < 12)
        cout << Dashboard::YELLOW << "  Warning: low credit load (" << totalC << " credits).\n" << Dashboard::RESET;
    if (totalC > 21)
        cout << Dashboard::RED << "  Warning: heavy credit load (" << totalC << " credits).\n" << Dashboard::RESET;

    FileManager::saveProfile(s);
    FileManager::saveCourses(courses);
    for (auto& c : courses) FileManager::saveMarks(c);

    cout << "\n  " << Dashboard::GREEN << "Setup complete!" << Dashboard::RESET << "\n";
    Dashboard::pause();
}

// ── Input Marks ───────────────────────────────────────────────────────────────
void inputMarks(Student& s, vector<Course>& courses) {
    Dashboard::clearScreen();
    Dashboard::printHeader("Input Marks");
    cout << "\n";
    for (int i=0;i<(int)courses.size();i++)
        cout << "  [" << i+1 << "] " << courses[i].name << "\n";
    int idx = getInt("\n  Select course (0=back): ", 0, (int)courses.size());
    if (idx==0) return;
    idx--;
    Course& c = courses[idx];

    bool isLab = false;
    cout << "\n  Assessment type:\n";
    if (!c.isLabOnly())
        cout << "  [1] Quiz  [2] Assignment  [3] Mid  [4] Final  [5] Project\n";
    if (c.hasLab)
        cout << "  [6] Practical  [7] Lab Mid  [8] Lab Final\n";
    cout << "  Choice: ";
    string tc; cin >> tc;

    string atype;
    if      (tc=="1") atype="Quiz";
    else if (tc=="2") atype="Assignment";
    else if (tc=="3") atype="Mid";
    else if (tc=="4") atype="Final";
    else if (tc=="5") atype="Project";
    else if (tc=="6") { atype="Practical";  isLab=true; }
    else if (tc=="7") { atype="Lab Mid";    isLab=true; }
    else if (tc=="8") { atype="Lab Final";  isLab=true; }
    else { cout << "  Invalid.\n"; Dashboard::pause(); return; }

    if (c.isLabOnly() && !isLab) {
        cout << "  Lab-only course — select a lab type.\n";
        Dashboard::pause(); return;
    }

    int num = c.nextNumber(atype, isLab);
    auto& marks = isLab ? c.labMarks : c.theoryMarks;

    // Duplicate check
    for (auto& a : marks) {
        if (a.type==atype && a.number==num) {
            cout << Dashboard::YELLOW << "  " << atype << " " << num << " already exists.\n" << Dashboard::RESET;
            string ow = getString("  Overwrite? (y/n): ");
            if (ow!="y"&&ow!="Y") return;
            marks.erase(remove_if(marks.begin(),marks.end(),
                [&](const Assessment& a){ return a.type==atype && a.number==num; }),
                marks.end());
            break;
        }
    }

    double total = getDouble("  " + atype + " " + to_string(num) + " out of how many marks? ", 1, 1000);

    double weight = 0;
    if      (atype=="Quiz")       weight = (c.quizCount>0)       ? c.quizW/c.quizCount : 0;
    else if (atype=="Assignment")  weight = (c.assignmentCount>0) ? c.assignmentW/c.assignmentCount : 0;
    else if (atype=="Mid")         weight = isLab ? c.labMidW   : c.midW;
    else if (atype=="Final")       weight = isLab ? c.labFinalW : c.finalW;
    else if (atype=="Project")     weight = isLab ? c.labProjectW : c.projectW;
    else if (atype=="Practical")   weight = (c.practicalCount>0) ? c.practicalW/c.practicalCount : 0;
    else if (atype=="Lab Mid")     weight = c.labMidW;
    else if (atype=="Lab Final")   weight = c.labFinalW;

    cout << "\n  Fill: " << Dashboard::CYAN << FileManager::bufferPath() << Dashboard::RESET << "\n";
    cout << "  Format: regno,marks  (one per line)\n";
    cout << "  Press Enter when done...";
    cin.ignore(numeric_limits<streamsize>::max(),'\n'); cin.get();

    string errMsg;
    auto rows = FileManager::readBufferFile(errMsg);
    if (!errMsg.empty()) {
        cout << Dashboard::RED << "  Error: " << errMsg << Dashboard::RESET << "\n";
        Dashboard::pause(); return;
    }
    if (rows.empty()) {
        cout << Dashboard::RED << "  Buffer file is empty.\n" << Dashboard::RESET;
        Dashboard::pause(); return;
    }

    double studentMark = -1;
    vector<double> allMarks;
    for (auto& [regno, mark] : rows) {
        allMarks.push_back(mark);
        if (regno == s.regno) studentMark = mark;
    }
    if (studentMark < 0) {
        cout << Dashboard::RED << "  Your regno (" << s.regno << ") not found.\n" << Dashboard::RESET;
        Dashboard::pause(); return;
    }
    if (studentMark > total) {
        cout << Dashboard::RED << "  Error: mark (" << studentMark << ") exceeds total (" << total << ").\n" << Dashboard::RESET;
        Dashboard::pause(); return;
    }

    Assessment a(atype, num, studentMark, total, weight);
    a = GradeEngine::calcClassStats(allMarks, studentMark, a);

    cout << "\n  " << Dashboard::BOLD << "Preview" << Dashboard::RESET << "\n";
    Dashboard::printLine('-',44);
    cout << "  Assessment : " << atype << " " << num << "\n";
    cout << "  Your mark  : " << studentMark << " / " << total << "\n";
    cout << "  Absolute   : " << fixed << setprecision(2) << a.absolute << "%\n";
    cout << "  Class mean : " << fixed << setprecision(1) << a.classMean << "\n";
    cout << "  Topper     : " << a.classTopper << "\n";
    cout << "  Your rank  : " << a.studentRank << " / " << a.classSize << "\n";
    if (s.gradingMode == "relative") {
        string relGrade = GradeEngine::calcRelativeGrade(
            a.absolute,
            (a.classMean / a.total) * a.weightage,
            (a.classStdDev / a.total) * a.weightage, s);
        cout << "  Relative grade for this assessment: "
             << Dashboard::MAGENTA << relGrade << Dashboard::RESET << "\n";
    }
    Dashboard::printLine('-',44);

    string confirm = getString("  Save? (y/n): ");
    if (confirm!="y"&&confirm!="Y") {
        cout << "  Cancelled. Buffer NOT wiped.\n";
        Dashboard::pause(); return;
    }

    marks.push_back(a);
    if (!FileManager::saveMarks(c)) {
        cout << Dashboard::RED << "  Write failed. Buffer NOT wiped.\n" << Dashboard::RESET;
        Dashboard::pause(); return;
    }
    FileManager::wipeBufferFile();
    cout << Dashboard::GREEN << "  Saved.\n" << Dashboard::RESET;
    Dashboard::pause();
}

// ── Select course ─────────────────────────────────────────────────────────────
int selectCourse(const vector<Course>& courses) {
    cout << "\n";
    for (int i=0;i<(int)courses.size();i++)
        cout << "  [" << i+1 << "] " << courses[i].name << "\n";
    cout << "  Select (0=back): ";
    int idx; cin >> idx;
    return idx - 1;
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    FileManager::createFolderStructure();

    Student s;
    vector<Course> courses;

    // Check for demo flag: run as ./study_manager demo
    bool demoArg = (argc > 1 && string(argv[1]) == "demo");
    if (demoArg) {
        loadDemo(s, courses);
    } else if (!FileManager::profileExists()) {
        runSetup(s, courses);
    } else {
        FileManager::loadProfile(s);
        FileManager::loadCourses(courses);
        for (auto& c : courses) FileManager::loadMarks(c);
        FileManager::loadHistory(s.history);
        if (courses.empty()) {
            cout << "\n  Starting new semester setup...\n";
            Dashboard::pause();
            runSetup(s, courses);
        }
    }

    string choice;
    while (true) {
        Dashboard::showMainMenu(s, courses);
        cin >> choice;

        if (choice == "0") { cout << "\n  Goodbye!\n\n"; break; }
        else if (choice=="1") { inputMarks(s, courses); }
        else if (choice=="2") {
            int idx = selectCourse(courses);
            if (idx>=0 && idx<(int)courses.size())
                Dashboard::showSubjectDashboard(courses[idx], s);
        }
        else if (choice=="3") { Dashboard::showSemesterSummary(courses, s); }
        else if (choice=="4") { Dashboard::showGradePredictions(courses, s); }
        else if (choice=="5") { Dashboard::showOverallAnalysis(courses, s); }
        else if (choice=="6") { Dashboard::showWhatIf(courses, s); }
        else if (choice=="7") { Dashboard::showMinToPass(courses, s); }
        else if (choice=="8") { Dashboard::showAttendance(courses); }
        else if (choice=="9") { Dashboard::showHistory(s); }
        else if (choice=="t"||choice=="T") { Dashboard::showTargetCGPA(s, courses); }
        else if (choice=="p"||choice=="P") { Dashboard::showPriorityList(courses, s); }
        else if (choice=="d"||choice=="D") { Dashboard::showDeadlineCalendar(courses); }
        else if (choice=="e"||choice=="E") { Dashboard::showExportSummary(s, courses); }
        else if (choice=="s"||choice=="S") {
            Settings::showMenu(s, courses);
            FileManager::saveProfile(s);
            FileManager::saveCourses(courses);
            if (courses.empty()) {
                cout << "\n  Starting new semester setup...\n";
                Dashboard::pause();
                runSetup(s, courses);
            }
        }
        // Hidden demo mode — type "demo" at the menu
        else if (choice=="demo") {
            loadDemo(s, courses);
        }
    }
    return 0;
}
