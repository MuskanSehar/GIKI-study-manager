#pragma once
#include "Student.h"
#include "Course.h"
#include "FileManager.h"
#include "GradeEngine.h"
#include <vector>
using namespace std;

class Settings {
public:
    static void showMenu(Student& s, vector<Course>& courses);

private:
    // Sub-sections
    static void courseSettings(Student& s, vector<Course>& courses);
    static void profileSettings(Student& s, vector<Course>& courses);
    static void gradeSettings(Student& s, vector<Course>& courses);
    static void semesterManagement(Student& s, vector<Course>& courses);

    // Course helpers
    static void editWeightage(Course& c, Student& s, vector<Course>& courses);
    static void addAssessment(Course& c, vector<Course>& courses);
    static void removeCourse(vector<Course>& courses, int idx);
    static void updateAttendance(Course& c, vector<Course>& courses);
    static void toggleDropLowest(Course& c, vector<Course>& courses);
};
