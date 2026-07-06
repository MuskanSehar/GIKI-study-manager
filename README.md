# GIKI Student Study Manager

A C++17 command-line academic tracking application built for **CS102 — Object
Oriented Programming Lab**, Spring 2026, GIKI Department of Computer
Engineering.

It turns raw marks into academic intelligence: absolute + relative grading,
class benchmarking, CGPA projection, and semester-to-semester history — all
persisted to plain text/CSV files, with zero external dependencies.

## Features

- **GIKI 4.0 grading scale** — absolute (fixed cutoffs) or relative
  (mean/SD-based curve) grading, fully customisable per semester
- **Class statistics engine** — mean, median, std deviation, variance, rank,
  and percentile computed from a pasted class result buffer
- **Grade & CGPA prediction** — best-case / worst-case grade per course,
  required SGPA to hit a target CGPA, and minimum-marks-to-pass gap analysis
- **Full data persistence** — one-time setup, then every mark entry is saved
  immediately; semesters archive automatically into history
- **Theory/lab credit separation**, attendance tracking, deadline calendar,
  study priority list, and a plain-text semester report generator

## Architecture

Eight classes, each owning one responsibility:

| Class | Responsibility |
|---|---|
| `Student` | Profile, grading mode, grade boundaries/points, semester history |
| `Course` | Per-course config (credits, weightages, counts) and marks |
| `Assessment` | A single quiz/mid/final/etc. entry + its class statistics |
| `GradeEngine` | All grading, GPA, prediction, and statistics math |
| `FileManager` | Reading/writing profile, courses, marks, buffer, history, reports |
| `Dashboard` | All read-only terminal views (menus, stats, predictions) |
| `Settings` | Interactive editing of courses, profile, grading, semester rollover |
| `main` | Wiring, first-time setup, input flow |

## Build & Run

Requires a C++17 compiler (GCC 8+, Clang 7+, MSVC 2017+) and CMake 3.16+.

```bash
mkdir build && cd build
cmake ..
make
./study_manager
```

On first run it walks you through setup (name, regno, courses, weightages).
All data is written under `data/`, `inbox/`, and `reports/`, created
automatically — these folders are gitignored since they hold your personal
academic data.

### Try it without entering your own data

```bash
./study_manager demo
```

Loads a synthetic 6-course, 45-student semester so you can explore every
dashboard immediately.

### Entering marks

The app never asks you to paste a whole class result into the terminal.
Instead: pick an assessment, then paste the full class result (one
`regno,marks` per line) into `inbox/input.txt`, and the app extracts your row,
computes class statistics from the rest, and wipes the buffer after reading.

## Grading model

```
absolute = (obtained / total) * weightage
SGPA = Σ(grade_points × credits) / Σ(credits)
CGPA = Σ(all semester totals) / Σ(all credits)
```

Relative grading maps a z-score `(studentAbs - classMean) / classSD` against
customisable thresholds (default: A ≥ +1.5 SD, B ≥ +0.5 SD, C ≥ -0.5 SD,
D ≥ -1.5 SD).

## Constraints (v1.0)

- Pure terminal interface, no GUI
- No network access — all data stored locally
- Repeat-course handling out of scope
- Manual data entry — each assessment requires pasting the class result
  into inbox/input.txt; no automatic import from GIKI's portal

## Contributors

This was built as a group project for CS102 OOP Lab.

- Abdullah Sultan (2025049)
- Muskan Sehar
- Saad Fida
- Musab Sial
- Mustafa Mudassar

## License

Coursework project — shared for portfolio purposes with permission from all
group members.
