#include "Dashboard.h"
#include "FileManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <ctime>
using namespace std;

const string Dashboard::RESET   = "\033[0m";
const string Dashboard::BOLD    = "\033[1m";
const string Dashboard::RED     = "\033[31m";
const string Dashboard::GREEN   = "\033[32m";
const string Dashboard::YELLOW  = "\033[33m";
const string Dashboard::CYAN    = "\033[36m";
const string Dashboard::MAGENTA = "\033[35m";
const string Dashboard::WHITE   = "\033[37m";
const string Dashboard::DIM     = "\033[2m";

void Dashboard::clearScreen() { cout << "\033[2J\033[H"; }
void Dashboard::printLine(char c, int width) { cout << string(width,c) << "\n"; }
void Dashboard::printHeader(const string& title) {
    printLine('=');
    int pad = max(0,(62-(int)title.size())/2);
    cout << string(pad,' ') << BOLD << CYAN << title << RESET << "\n";
    printLine('=');
}
void Dashboard::pause() {
    cout << DIM << "\nPress Enter to continue..." << RESET;
    cin.ignore(1000,'\n'); cin.get();
}

// ── Health Snapshot ───────────────────────────────────────────────────────────
void Dashboard::showHealthSnapshot(const Student& s, const vector<Course>& courses) {
    cout << "\n" << BOLD << "  Health Snapshot" << RESET << "\n";
    printLine('-',62);

    // Credit summary
    int semCreds = GradeEngine::totalSemCredits(courses);
    cout << "  " << DIM << "Credits this semester: " << semCreds << RESET << "\n";
    if (semCreds < 12) cout << "  " << YELLOW << "  Low credit load (< 12)" << RESET << "\n";
    if (semCreds > 21) cout << "  " << RED    << "  Heavy credit load (> 21)" << RESET << "\n";

    // CGPA
    double predSGPA = GradeEngine::calcSemesterSGPA(courses, s);
    double projCGPA = GradeEngine::calcProjectedCGPA(s, predSGPA, semCreds);
    if (s.currentSemester == 1) {
        cout << "  " << BOLD << "Predicted SGPA : " << RESET
             << CYAN << fixed << setprecision(2) << predSGPA << RESET << "\n";
    } else {
        cout << "  " << BOLD << "Finalized CGPA : " << RESET
             << GREEN << fixed << setprecision(2) << s.previousCGPA << RESET
             << "   |   " << BOLD << "Projected CGPA : " << RESET
             << CYAN  << fixed << setprecision(2) << projCGPA << RESET << "\n";
    }
    if (projCGPA < 2.0 && semCreds > 0)
        cout << "  " << RED << BOLD << "! PROBATION RISK — Projected CGPA below 2.00 !" << RESET << "\n";

    // Pending assessments
    int pending = 0;
    for (auto& c : courses) {
        int exp = c.quizCount + c.assignmentCount + 1 + 1;
        if (c.hasProject) exp++;
        pending += max(0, exp - (int)c.theoryMarks.size());
        if (c.hasLab) {
            int labExp = c.practicalCount + 1;
            if (c.labHasProject) labExp++;
            pending += max(0, labExp - (int)c.labMarks.size());
        }
    }
    if (pending > 0)
        cout << "  " << YELLOW << pending << " assessment(s) pending entry" << RESET << "\n";

    // Teacher avg reminder for relative courses
    for (auto& c : courses) {
        if (s.gradingMode == "relative" && s.teacherAvg < 0)
            cout << "  " << MAGENTA << "Reminder: set teacher average for " << c.name
                 << " (Settings > Course Settings)" << RESET << "\n";
    }

    // At-risk
    for (auto& c : courses) {
        const auto& m = c.isLabOnly() ? c.labMarks : c.theoryMarks;
        double abs  = GradeEngine::calcRunningAbsolute(m);
        double pace = GradeEngine::calcPace(m);
        if (!m.empty() && pace < 70.0)
            cout << "  " << RED << "At-risk: " << c.name
                 << " (" << fixed << setprecision(1) << abs << "% so far, pace "
                 << fixed << setprecision(1) << pace << "%)" << RESET << "\n";
    }

    // Attendance warnings
    for (auto& c : courses) {
        if (c.totalClasses > 0) {
            double pct = 100.0 * c.attended / c.totalClasses;
            if (pct < 75.0)
                cout << "  " << YELLOW << "Low attendance: " << c.name
                     << " (" << fixed << setprecision(0) << pct << "%)" << RESET << "\n";
        }
    }

    // Upcoming deadlines (next 7 days)
    for (auto& c : courses) {
        for (auto& [label, date] : c.upcomingDates) {
            if (!date.empty())
                cout << "  " << CYAN << "Upcoming: " << c.name << " — " << label
                     << " on " << date << RESET << "\n";
        }
    }

    printLine('-',62);
}

// ── Main Menu ─────────────────────────────────────────────────────────────────
void Dashboard::showMainMenu(const Student& s, const vector<Course>& courses) {
    clearScreen();
    printHeader("GIKI Student Study Manager");
    cout << "  " << DIM << s.name << "  |  " << s.sessionLabel
         << "  |  Semester " << s.currentSemester << RESET << "\n";
    showHealthSnapshot(s, courses);
    cout << "\n";
    cout << "  " << BOLD << "[1]" << RESET << " Input Marks\n";
    cout << "  " << BOLD << "[2]" << RESET << " Subject Dashboard\n";
    cout << "  " << BOLD << "[3]" << RESET << " Semester Summary\n";
    cout << "  " << BOLD << "[4]" << RESET << " Grade Predictions\n";
    cout << "  " << BOLD << "[5]" << RESET << " Overall Analysis\n";
    cout << "  " << BOLD << "[6]" << RESET << " What-If Simulator\n";
    cout << "  " << BOLD << "[7]" << RESET << " Minimum-to-Pass\n";
    cout << "  " << BOLD << "[8]" << RESET << " Attendance\n";
    cout << "  " << BOLD << "[9]" << RESET << " Semester History\n";
    cout << "  " << BOLD << "[T]" << RESET << " Target CGPA\n";
    cout << "  " << BOLD << "[P]" << RESET << " Study Priority List\n";
    cout << "  " << BOLD << "[D]" << RESET << " Deadline Calendar\n";
    cout << "  " << BOLD << "[E]" << RESET << " Export Marks Summary\n";
    cout << "  " << BOLD << "[S]" << RESET << " Settings\n";
    cout << "  " << BOLD << "[0]" << RESET << " Exit\n";
    printLine('=');
    cout << "  Choice: ";
}

// ── Subject Dashboard ─────────────────────────────────────────────────────────
void Dashboard::showSubjectDashboard(const Course& c, const Student& s) {
    clearScreen();
    printHeader(c.name + " — Dashboard");
    cout << "  Grading: " << BOLD
         << (s.gradingMode=="relative" ? MAGENTA+"Relative (Normal Dist)" : CYAN+"Absolute")
         << RESET << "\n";

    auto printSection = [&](const vector<Assessment>& marks, bool isLab) {
        string label = isLab ? "LAB" : "THEORY";
        cout << "\n  " << BOLD << label << " MARKS" << RESET << "\n";
        if (marks.empty()) {
            cout << "  " << DIM << "No marks entered yet.\n" << RESET;
            return;
        }
        cout << "  " << left
             << setw(14) << "Assessment" << setw(10) << "Got/Total"
             << setw(8)  << "Weight"     << setw(8)  << "Abs%"
             << setw(8)  << "Rank"       << setw(12) << "vs Mean" << "\n";
        printLine('-',62);
        for (auto& a : marks) {
            cout << "  " << left
                 << setw(14) << (a.type+" "+to_string(a.number))
                 << setw(10) << (to_string((int)a.obtained)+"/"+to_string((int)a.total))
                 << setw(8)  << fixed << setprecision(1) << a.weightage
                 << setw(8)  << fixed << setprecision(2) << a.absolute;
            if (a.hasClassStats) {
                cout << setw(8) << (to_string(a.studentRank)+"/"+to_string(a.classSize));
                string abvCol = a.marksAboveMean >= 0 ? GREEN : RED;
                cout << abvCol << fixed << setprecision(1)
                     << (a.marksAboveMean >= 0 ? "+" : "") << a.marksAboveMean << RESET;
            } else {
                cout << setw(8) << "N/A" << DIM << "N/A" << RESET;
            }
            cout << "\n";
        }
        printLine('-',62);

        double running = GradeEngine::calcRunningAbsolute(marks);
        double remain  = GradeEngine::calcRemainingWeightage(marks);
        string grade   = GradeEngine::getCourseGrade(c, s, isLab);
        string best    = GradeEngine::calcBestCase(c, s, isLab);
        string worst   = GradeEngine::calcWorstCase(c, s, isLab);

        cout << "  Running Absolute : " << CYAN << fixed << setprecision(2) << running << "%" << RESET << "\n";
        cout << "  Current Grade    : " << BOLD << CYAN << grade << RESET;
        if (s.gradingMode == "relative") cout << "  " << DIM << "(relative)" << RESET;
        cout << "\n";
        cout << "  Best / Worst     : " << GREEN << best << RESET << " / " << RED << worst << RESET << "\n";

        if (remain > 60)
            cout << "  " << YELLOW << "[Low confidence — " << fixed << setprecision(0)
                 << remain << "% weightage untested]" << RESET << "\n";

        // Trend
        if (!isLab) {
            for (auto& type : {"Quiz","Assignment"}) {
                string trend = GradeEngine::calcTrend(marks, type);
                if (trend != "N/A") {
                    string col = (trend=="Improving") ? GREEN : (trend=="Declining") ? RED : YELLOW;
                    cout << "  " << type << " trend: " << col << trend << RESET << "\n";
                }
            }
        }

        // Quiz heatmap
        bool hasQuizStats = false;
        for (auto& a : marks) if (a.type=="Quiz" && a.hasClassStats) { hasQuizStats=true; break; }
        if (hasQuizStats) {
            cout << "  Quiz heatmap: ";
            for (auto& a : marks) {
                if (a.type != "Quiz") continue;
                if (a.total <= 0) continue;
                double eff = a.obtained / a.total;
                double classEff = a.classMean / a.total;
                string blk = (eff > classEff+0.05) ? GREEN+"█"+RESET
                           : (eff < classEff-0.05) ? RED+"█"+RESET
                           :                         YELLOW+"█"+RESET;
                cout << blk;
            }
            cout << "  (green=above mean, red=below)\n";
        }

        // Target gap
        double needed = GradeEngine::calcRequiredInRemaining(c, c.targetGrade, s, isLab);
        cout << "  Target: " << BOLD << c.targetGrade << RESET;
        if (needed <= 0)   cout << "  " << GREEN  << "(Already achieved!)" << RESET;
        else if (needed > 100) cout << "  " << RED << "(No longer achievable)" << RESET;
        else cout << "  Need " << YELLOW << fixed << setprecision(1) << needed << "% in remaining" << RESET;
        cout << "\n";

        // Class stats table
        bool anyStats = false;
        for (auto& a : marks) if (a.hasClassStats) { anyStats=true; break; }
        if (anyStats) {
            cout << "\n  " << BOLD << "CLASS STATISTICS" << RESET << "\n";
            printLine('-',62);
            cout << "  " << left << setw(14) << "Assessment"
                 << setw(8) << "Mean" << setw(8) << "Median"
                 << setw(8) << "SD"   << setw(8) << "Topper"
                 << setw(8) << "Pctile" << "\n";
            printLine('-',62);
            for (auto& a : marks) {
                if (!a.hasClassStats) continue;
                cout << "  " << left
                     << setw(14) << (a.type+" "+to_string(a.number))
                     << setw(8)  << fixed << setprecision(1) << a.classMean
                     << setw(8)  << fixed << setprecision(1) << a.classMedian
                     << setw(8)  << fixed << setprecision(1) << a.classStdDev
                     << setw(8)  << fixed << setprecision(1) << a.classTopper
                     << setw(8)  << fixed << setprecision(1) << a.studentPercentile << "\n";
            }
        }
    };

    if (!c.isLabOnly()) printSection(c.theoryMarks, false);
    if (c.hasLab)       printSection(c.labMarks, true);
    pause();
}

// ── Semester Summary ──────────────────────────────────────────────────────────
void Dashboard::showSemesterSummary(const vector<Course>& courses, const Student& s) {
    clearScreen();
    printHeader("Semester Summary");
    cout << "\n  " << left
         << setw(20) << "Course" << setw(8) << "Abs%"
         << setw(8)  << "Grade"  << setw(8) << "Mode" << setw(6) << "Creds" << "\n";
    printLine('-',62);
    int semCredits = 0;
    for (auto& c : courses) {
        bool labOnly = c.isLabOnly();
        const auto& m = labOnly ? c.labMarks : c.theoryMarks;
        double abs    = GradeEngine::calcRunningAbsolute(m);
        string grade  = m.empty() ? "N/A" : GradeEngine::getCourseGrade(c, s, labOnly);
        int creds     = c.theoryCredits + c.labCredits;
        semCredits   += creds;
        string modeTag = (s.gradingMode=="relative") ? "Rel" : "Abs";
        cout << "  " << left
             << setw(20) << c.name
             << setw(8)  << (m.empty() ? "N/A" : (to_string((int)abs)+"%"))
             << setw(8)  << grade
             << setw(8)  << modeTag
             << setw(6)  << creds << "\n";
    }
    printLine('-',62);
    double predSGPA = GradeEngine::calcSemesterSGPA(courses, s);
    double projCGPA = GradeEngine::calcProjectedCGPA(s, predSGPA, semCredits);
    cout << "\n  Total Credits    : " << BOLD << semCredits << RESET << "\n";
    cout << "  Predicted SGPA   : " << CYAN << BOLD << fixed << setprecision(2) << predSGPA << RESET << "\n";
    if (s.currentSemester > 1) {
        cout << "  Finalized CGPA   : " << GREEN << fixed << setprecision(2) << s.previousCGPA << RESET << "\n";
        cout << "  Projected CGPA   : " << CYAN  << fixed << setprecision(2) << projCGPA << RESET << "\n";
    }
    pause();
}

// ── Grade Predictions ─────────────────────────────────────────────────────────
void Dashboard::showGradePredictions(const vector<Course>& courses, const Student& s) {
    clearScreen();
    printHeader("Grade Predictions");
    for (auto& c : courses) {
        cout << "\n  " << BOLD << c.name << RESET;
        if (s.gradingMode=="relative") cout << "  " << MAGENTA << "[Relative]" << RESET;
        cout << "\n";
        const auto& m = c.isLabOnly() ? c.labMarks : c.theoryMarks;
        if (m.empty()) { cout << "  " << DIM << "No marks — N/A\n" << RESET; continue; }
        double abs  = GradeEngine::calcRunningAbsolute(m);
        string pred = GradeEngine::getCourseGrade(c, s, c.isLabOnly());
        string best = GradeEngine::calcBestCase(c, s, c.isLabOnly());
        string worst= GradeEngine::calcWorstCase(c, s, c.isLabOnly());
        double rem  = GradeEngine::calcRemainingWeightage(m);
        cout << "  Predicted: " << CYAN << BOLD << pred << RESET
             << "   Best: " << GREEN << best << RESET
             << "   Worst: " << RED << worst << RESET << "\n";
        if (rem > 60)
            cout << "  " << YELLOW << "[Low confidence — " << fixed << setprecision(0) << rem << "% untested]" << RESET << "\n";
        if (best == worst)
            cout << "  " << GREEN << "Grade " << best << " is guaranteed." << RESET << "\n";
        // Grade improvement suggestion
        string curGrade = s.getGrade(abs);
        // Suggest what improvement in weakest type would do
        string weakType = "";
        double weakEff  = 2.0;
        for (auto& a : m) {
            if (a.total > 0) {
                double eff = a.obtained / a.total;
                if (eff < weakEff) { weakEff = eff; weakType = a.type; }
            }
        }
        if (!weakType.empty() && !m.empty()) {
            cout << "  " << DIM << "Improving " << weakType << " scores by ~15%"
                 << " could boost your grade." << RESET << "\n";
        }
    }
    pause();
}

// ── Overall Analysis ──────────────────────────────────────────────────────────
void Dashboard::showOverallAnalysis(const vector<Course>& courses, const Student& s) {
    clearScreen();
    printHeader("Overall Analysis");
    bool anyMarks = false;
    for (auto& c : courses)
        if (!c.theoryMarks.empty() || !c.labMarks.empty()) { anyMarks=true; break; }
    if (!anyMarks) {
        cout << "\n  " << DIM << "No marks entered yet.\n" << RESET;
        pause(); return;
    }

    // Sort weakest first
    vector<const Course*> sorted;
    for (auto& c : courses) sorted.push_back(&c);
    sort(sorted.begin(), sorted.end(), [](const Course* a, const Course* b){
        auto getAbs = [](const Course* c){
            return GradeEngine::calcRunningAbsolute(c->isLabOnly() ? c->labMarks : c->theoryMarks);
        };
        return getAbs(a) < getAbs(b);
    });

    cout << "\n  Courses ranked weakest → strongest:\n\n";
    for (auto* cp : sorted) {
        const auto& weakMarks = cp->isLabOnly() ? cp->labMarks : cp->theoryMarks;
        double abs  = GradeEngine::calcRunningAbsolute(weakMarks);
        double pace = GradeEngine::calcPace(weakMarks);
        string grade= GradeEngine::getCourseGrade(*cp, s, cp->isLabOnly());
        string col  = (pace < 60) ? RED : (pace < 75) ? YELLOW : GREEN;
        cout << "  " << col << BOLD << setw(22) << left << cp->name << RESET
             << col << fixed << setprecision(1) << abs << "%" << RESET
             << "  (" << grade << ")";
        if (pace < 70) cout << "  " << RED << "< At Risk" << RESET;

        // Retake warning
        double minNeeded = GradeEngine::calcMinToPass(*cp, s, "D", cp->isLabOnly());
        if (minNeeded > 100)
            cout << "  " << RED << BOLD << "RETAKE LIKELY" << RESET;
        cout << "\n";

        // Weakest component
        string weakType = "";
        double weakEff  = 2.0;
        for (auto& a : weakMarks) {
            if (a.total > 0) {
                double eff = a.obtained / a.total;
                if (eff < weakEff) { weakEff = eff; weakType = a.type; }
            }
        }
        if (!weakType.empty())
            cout << "  " << DIM << "  Weakest: " << weakType
                 << " (" << fixed << setprecision(0) << (weakEff*100) << "%)" << RESET << "\n";
    }

    double predSGPA = GradeEngine::calcSemesterSGPA(courses, s);
    cout << "\n  " << BOLD << "Predicted Standing: " << RESET;
    if      (predSGPA >= 3.5) cout << GREEN  << "Distinction" << RESET;
    else if (predSGPA >= 3.0) cout << CYAN   << "Merit" << RESET;
    else if (predSGPA >= 2.0) cout << YELLOW << "Satisfactory" << RESET;
    else                      cout << RED    << "Probation Risk" << RESET;
    cout << " (SGPA: " << fixed << setprecision(2) << predSGPA << ")\n";
    pause();
}

// ── What-If Simulator ─────────────────────────────────────────────────────────
void Dashboard::showWhatIf(const vector<Course>& courses, const Student& s) {
    clearScreen();
    printHeader("What-If Simulator");
    cout << "\n  " << DIM << "Nothing will be saved.\n" << RESET << "\n";
    for (int i=0;i<(int)courses.size();i++)
        cout << "  [" << i+1 << "] " << courses[i].name << "\n";
    cout << "  Course: "; int idx; cin >> idx; idx--;
    if (idx < 0 || idx >= (int)courses.size()) { pause(); return; }

    const Course& c = courses[idx];
    bool labOnly = c.isLabOnly();
    const auto& m = labOnly ? c.labMarks : c.theoryMarks;
    double running = GradeEngine::calcRunningAbsolute(m);
    double remain  = GradeEngine::calcRemainingWeightage(m);

    cout << "  Current absolute : " << fixed << setprecision(2) << running << "%\n";
    cout << "  Remaining weight : " << fixed << setprecision(2) << remain  << "%\n";
    cout << "  Obtained: "; double obt; cin >> obt;
    cout << "  Out of  : "; double tot; cin >> tot;
    if (tot <= 0) { cout << RED << "  Invalid.\n" << RESET; pause(); return; }
    if (obt > tot) cout << RED << "  Obtained > total — capped.\n" << RESET;
    obt = min(obt, tot);

    double hypAbs   = running + GradeEngine::calcAbsolute(obt, tot, remain);
    string hypGrade = s.getGrade(hypAbs);

    vector<Course> tmp = courses;
    Assessment hyp("Hyp",1,obt,tot,remain);
    hyp.absolute = GradeEngine::calcAbsolute(obt,tot,remain);
    (labOnly ? tmp[idx].labMarks : tmp[idx].theoryMarks).push_back(hyp);

    double hypSGPA = GradeEngine::calcSemesterSGPA(tmp, s);
    int    semCreds= GradeEngine::totalSemCredits(tmp);
    double hypCGPA = GradeEngine::calcProjectedCGPA(s, hypSGPA, semCreds);

    cout << "\n  " << BOLD << "Hypothetical Results:" << RESET << "\n";
    printLine('-',40);
    cout << "  Grade     : " << CYAN << BOLD << hypGrade << RESET << "\n";
    cout << "  Abs %     : " << CYAN << fixed << setprecision(2) << hypAbs << "%" << RESET << "\n";
    cout << "  SGPA      : " << CYAN << fixed << setprecision(2) << hypSGPA << RESET << "\n";
    if (s.currentSemester > 1)
        cout << "  Proj CGPA : " << CYAN << fixed << setprecision(2) << hypCGPA << RESET << "\n";

    // Estimated class rank using stored distribution
    bool hasStats = false;
    double lastMean=0, lastSD=0, lastTotal=0;
    for (auto& a : m) if (a.hasClassStats) { hasStats=true; lastMean=a.classMean; lastSD=a.classStdDev; lastTotal=a.total; }
    if (hasStats && lastTotal > 0 && lastSD > 0) {
        double normMark = obt/tot * lastTotal;
        double z = (normMark - lastMean) / lastSD;
        cout << "  Est. Z-score: " << CYAN << fixed << setprecision(2) << z << RESET
             << "  (+" << fixed << setprecision(1) << (obt/tot*100 - lastMean/lastTotal*100)
             << "% vs class mean)\n";
    } else {
        cout << "  Class Rank: " << DIM << "N/A (no class data yet)" << RESET << "\n";
    }
    pause();
}

// ── Minimum to Pass ───────────────────────────────────────────────────────────
void Dashboard::showMinToPass(const vector<Course>& courses, const Student& s) {
    clearScreen();
    printHeader("Minimum-to-Pass Calculator");
    cout << "\n";
    bool allSecured = true;
    for (auto& c : courses) {
        bool labOnly = c.isLabOnly();
        double minNeeded = GradeEngine::calcMinToPass(c, s, "D", labOnly);
        cout << "  " << BOLD << setw(22) << left << c.name << RESET;
        if (minNeeded <= 0) {
            cout << GREEN << "Passing secured." << RESET << "\n";
        } else if (minNeeded > 100) {
            cout << RED << BOLD << "IMPOSSIBLE — retake likely." << RESET << "\n";
            allSecured = false;
        } else {
            cout << YELLOW << "Need " << fixed << setprecision(1) << minNeeded << "% in remaining." << RESET << "\n";
            allSecured = false;
        }
    }
    if (allSecured)
        cout << "\n  " << GREEN << BOLD << "All courses passing secured." << RESET << "\n";
    pause();
}

// ── Attendance ────────────────────────────────────────────────────────────────
void Dashboard::showAttendance(const vector<Course>& courses) {
    clearScreen();
    printHeader("Attendance");
    cout << "\n  " << left << setw(22) << "Course" << setw(12) << "Attended"
         << setw(12) << "Total" << "Percent\n";
    printLine('-',62);
    for (auto& c : courses) {
        double pct = c.totalClasses > 0 ? 100.0*c.attended/c.totalClasses : 0;
        string col = (pct >= 75) ? GREEN : RED;
        cout << "  " << left << setw(22) << c.name << setw(12) << c.attended
             << setw(12) << c.totalClasses
             << col << fixed << setprecision(1) << pct << "%" << RESET;
        if (pct < 75 && c.totalClasses > 0) cout << "  " << RED << "! Below 75%" << RESET;
        cout << "\n";
    }
    pause();
}

// ── Semester History ──────────────────────────────────────────────────────────
void Dashboard::showHistory(const Student& s) {
    clearScreen();
    printHeader("Semester History");
    if (s.history.empty()) {
        cout << "\n  " << DIM << "No completed semesters yet.\n" << RESET;
        pause(); return;
    }
    cout << "\n  " << left << setw(20) << "Session" << setw(12) << "Semester" << "SGPA\n";
    printLine('-',44);
    for (auto& r : s.history)
        cout << "  " << left << setw(20) << r.sessionLabel << setw(12) << r.semesterNumber
             << CYAN << fixed << setprecision(2) << r.sgpa << RESET << "\n";
    pause();
}

// ── Target CGPA ───────────────────────────────────────────────────────────────
void Dashboard::showTargetCGPA(const Student& s, const vector<Course>& courses) {
    clearScreen();
    printHeader("Target CGPA Calculator");
    cout << "\n  Enter desired CGPA (0.00–4.00): ";
    double target; cin >> target;
    if (target < 0 || target > 4.0) { cout << RED << "  Invalid.\n" << RESET; pause(); return; }
    if (target <= s.previousCGPA && s.currentSemester > 1) {
        cout << GREEN << "\n  Already achieved — current CGPA: "
             << fixed << setprecision(2) << s.previousCGPA << "\n" << RESET;
        pause(); return;
    }
    double req = GradeEngine::calcRequiredSGPA(s, target);
    if (req > 4.0) {
        int ec = 18;
        double maxP = GradeEngine::calcProjectedCGPA(s, 4.0, ec);
        cout << RED << "\n  Not achievable. Max possible: " << fixed << setprecision(2) << maxP << "\n" << RESET;
    } else {
        cout << "\n  To reach CGPA " << CYAN << fixed << setprecision(2) << target << RESET
             << " you need SGPA: " << BOLD << GREEN << fixed << setprecision(2) << req << RESET << "\n";
        // Suggested grades per course
        auto suggestions = GradeEngine::suggestCourseGrades(s, courses, target);
        cout << "\n  Suggested grades per course:\n";
        for (auto& [name, grade] : suggestions)
            cout << "  " << setw(22) << left << name << CYAN << grade << RESET << "\n";
    }
    pause();
}

// ── Study Priority List ───────────────────────────────────────────────────────
void Dashboard::showPriorityList(const vector<Course>& courses, const Student& s) {
    clearScreen();
    printHeader("Study Priority List");
    cout << "\n  " << DIM << "Higher score = study this first\n" << RESET << "\n";

    vector<pair<double,const Course*>> scores;
    for (auto& c : courses)
        scores.push_back({GradeEngine::calcPriorityScore(c, s), &c});
    sort(scores.begin(), scores.end(), [](auto& a, auto& b){ return a.first > b.first; });

    int rank = 1;
    for (auto& [score, cp] : scores) {
        string col = (rank==1) ? RED : (rank==2) ? YELLOW : GREEN;
        cout << "  " << col << BOLD << rank << "." << RESET << "  "
             << setw(22) << left << cp->name
             << "  Priority score: " << col << fixed << setprecision(1) << score << RESET << "\n";
        // Trend
        string trend = GradeEngine::calcTrend(cp->theoryMarks, "Quiz");
        if (trend != "N/A") {
            string tc = (trend=="Improving") ? GREEN : (trend=="Declining") ? RED : YELLOW;
            cout << "     Quiz trend: " << tc << trend << RESET << "\n";
        }
        rank++;
    }
    pause();
}

// ── Deadline Calendar ─────────────────────────────────────────────────────────
void Dashboard::showDeadlineCalendar(const vector<Course>& courses) {
    clearScreen();
    printHeader("Deadline Calendar");
    bool any = false;
    for (auto& c : courses) {
        if (c.upcomingDates.empty()) continue;
        any = true;
        cout << "\n  " << BOLD << c.name << RESET << "\n";
        for (auto& [label, date] : c.upcomingDates)
            cout << "  " << CYAN << setw(20) << left << label << RESET << "  " << date << "\n";
    }
    if (!any) cout << "\n  " << DIM << "No deadlines set. Add them in Settings > Course Settings.\n" << RESET;
    pause();
}

// ── Export Marks Summary ──────────────────────────────────────────────────────
void Dashboard::showExportSummary(const Student& s, const vector<Course>& courses) {
    clearScreen();
    printHeader("Export Marks Summary");

    time_t now2 = time(nullptr);
    tm* t = localtime(&now2);
    char dateBuf[32]; strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d_%H%M", t);
    string filename = "reports/marks_export_" + string(dateBuf) + ".csv";

    ofstream f(filename);
    if (!f.is_open()) {
        cout << RED << "\n  Could not create export file.\n" << RESET;
        pause(); return;
    }
    f << "Student," << s.name << "\nRegno," << s.regno << "\nSession," << s.sessionLabel << "\n\n";
    f << "Course,Type,Number,Obtained,Total,Weightage,Absolute,Rank,ClassSize,Percentile\n";
    for (auto& c : courses) {
        for (auto& a : c.theoryMarks)
            f << c.name << "," << a.type << "," << a.number << ","
              << a.obtained << "," << a.total << "," << a.weightage << "," << a.absolute << ","
              << a.studentRank << "," << a.classSize << "," << fixed << setprecision(1) << a.studentPercentile << "\n";
        for (auto& a : c.labMarks)
            f << c.name << "(Lab)," << a.type << "," << a.number << ","
              << a.obtained << "," << a.total << "," << a.weightage << "," << a.absolute << ","
              << a.studentRank << "," << a.classSize << "," << fixed << setprecision(1) << a.studentPercentile << "\n";
    }
    cout << GREEN << "\n  Exported to: " << filename << RESET << "\n";
    pause();
}

// ── Credit Summary ────────────────────────────────────────────────────────────
void Dashboard::showCreditSummary(const vector<Course>& courses) {
    int total = 0;
    for (auto& c : courses) total += c.theoryCredits + c.labCredits;
    // shown inline in health snapshot — no separate view needed
}
