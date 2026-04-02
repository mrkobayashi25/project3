#include "CampusCompass.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

using namespace std;

CampusCompass::CampusCompass() {
    // come back later
}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    // clear old data in case this gets called again
    campusGraph.clear();
    classes.clear();
    studentRecords.clear();

    // try loading both csv files
    bool loadedEdges = LoadEdgeData(edges_filepath);
    bool loadedCourses = LoadCourseData(classes_filepath);

    // if either one fails, return false
    if (!loadedEdges || !loadedCourses) {
        return false;
    }

    // print temp counts to make sure it loaded right
    PrintLoadSummary();
    return true;
}

bool CampusCompass::LoadEdgeData(const string& filename) {
    ifstream inputFile(filename);

    // make sure file opened
    if (!inputFile.is_open()) {
        return false;
    }

    string line;
    getline(inputFile, line); // skip header row

    while (getline(inputFile, line)) {
        // skip blank lines if there are any
        if (line.empty()) {
            continue;
        }

        // break current csv row into pieces
        stringstream lineStream(line);
        string location1Str;
        string location2Str;
        string name1;
        string name2;
        string travelTimeStr;

        getline(lineStream, location1Str, ',');
        getline(lineStream, location2Str, ',');
        getline(lineStream, name1, ',');
        getline(lineStream, name2, ',');
        getline(lineStream, travelTimeStr, ',');

        // skip bad / incomplete rows
        if (location1Str.empty() || location2Str.empty() || travelTimeStr.empty()) {
            continue;
        }

        // convert strings
        int location1 = stoi(location1Str);
        int location2 = stoi(location2Str);
        int travelTime = stoi(travelTimeStr);

        // edge from first location to second
        CampusEdge edgeToSecond;
        edgeToSecond.adjacentId = location2;
        edgeToSecond.travelTime = travelTime;
        edgeToSecond.available = true;

        // edge from second location back to first
        CampusEdge edgeToFirst;
        edgeToFirst.adjacentId = location1;
        edgeToFirst.travelTime = travelTime;
        edgeToFirst.available = true;

        // store both directions since graph is undirected
        campusGraph[location1].push_back(edgeToSecond);
        campusGraph[location2].push_back(edgeToFirst);
    }

    inputFile.close();
    return true;
}

bool CampusCompass::LoadCourseData(const string& filename) {
    ifstream inputFile(filename);

    // make sure file opened
    if (!inputFile.is_open()) {
        return false;
    }

    string line;
    getline(inputFile, line); // skip header

    while (getline(inputFile, line)) {
        // skip blank lines
        if (line.empty()) {
            continue;
        }

        // break current csv row to bits
        stringstream lineStream(line);
        string classCode;
        string classLocationStr;
        string startTime;
        string endTime;

        getline(lineStream, classCode, ',');
        getline(lineStream, classLocationStr, ',');
        getline(lineStream, startTime, ',');
        getline(lineStream, endTime, ',');

        // skip bad / incomplete rows
        if (classCode.empty() || classLocationStr.empty()) {
            continue;
        }

        // make a course object for current row
        CourseInfo currentCourse;
        currentCourse.classLocation = stoi(classLocationStr);
        currentCourse.startTime = startTime;
        currentCourse.endTime = endTime;

        // map class code to course info
        classes[classCode] = currentCourse;
    }

    inputFile.close();
    return true;
}

void CampusCompass::PrintLoadSummary() const {
    int edgeCount = 0;

    // count stored edges in list
    for (const auto& location : campusGraph) {
        edgeCount += location.second.size();
    }

    // divide by 2
    edgeCount /= 2;

    // temp debug output for csv loading
    cout << "locations loaded: " << campusGraph.size() << endl;
    cout << "edges loaded: " << edgeCount << endl;
    cout << "class codes loaded: " << classes.size() << endl;
}

bool CampusCompass::ValidUFID(const string& ufid) const {
    // ufid has to be 8 digits
    if (ufid.length() != 8) {
        return false;
    }

    // make sure every char is number
    for (char ch : ufid) {
        if (!isdigit(ch)) {
            return false;
        }
    }

    return true;
}

bool CampusCompass::ValidName(const string& name) const {
    // empty name should fail
    if (name.empty()) {
        return false;
    }

    // only letters / spaces allowed
    for (char ch : name) {
        if (!isalpha(ch) && ch != ' ') {
            return false;
        }
    }

    return true;
}

bool CampusCompass::ValidClassCode(const string& code) const {
    // class code should be 7 char
    if (code.length() != 7) {
        return false;
    }

    // first 3 should be uppercase letters
    for (int i = 0; i < 3; i++) {
        if (!isupper(code[i])) {
            return false;
        }
    }

    // last 4 should be digits
    for (int i = 3; i < 7; i++) {
        if (!isdigit(code[i])) {
            return false;
        }
    }

    return true;
}

bool CampusCompass::ValidClassCount(int count) const {
    // class count has to be between 1 and 6
    return count >= 1 && count <= 6;
}

bool CampusCompass::ParseInsertCommand(const string& command, string& studentName, string& ufid, int& residenceLocation,
                                       vector<string>& classCodes) const {
    // clear old class codes first
    classCodes.clear();

    // find quoted name
    size_t firstQuote = command.find('"');
    size_t secondQuote = command.find('"', firstQuote + 1);

    // if quotes are missing or messed up, fail
    if (firstQuote == string::npos || secondQuote == string::npos || secondQuote <= firstQuote) {
        return false;
    }

    // pull out name between quotes
    studentName = command.substr(firstQuote + 1, secondQuote - firstQuote - 1);

    // everything after quoted name
    string remaining = command.substr(secondQuote + 1);
    stringstream lineStream(remaining);

    int classCount;

    // read ufid, residence location, and class count
    if (!(lineStream >> ufid >> residenceLocation >> classCount)) {
        return false;
    }

    // make sure class count is valid
    if (!ValidClassCount(classCount)) {
        return false;
    }

    // read class codes
    string currentCode;
    while (lineStream >> currentCode) {
        classCodes.push_back(currentCode);
    }

    // make sure # of class codes matches count given
    if (classCodes.size() != static_cast<size_t>(classCount)) {
        return false;
    }

    return true;
}

bool CampusCompass::ParseCommand(const string &command) {
    // only doing insert rn
    if (command.rfind("insert ", 0) == 0) {
        string studentName;
        string ufid;
        int residenceLocation;
        vector<string> classCodes;

        // first make sure insert format parses right
        bool parsed = ParseInsertCommand(command, studentName, ufid, residenceLocation, classCodes);

        if (!parsed) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // make sure name and ufid are valid
        if (!ValidName(studentName) || !ValidUFID(ufid)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // ufid cant already exist
        if (studentRecords.find(ufid) != studentRecords.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // make sure each class code is valid and exists
        for (const string& code : classCodes) {
            if (!ValidClassCode(code) || classes.find(code) == classes.end()) {
                cout << "unsuccessful" << endl;
                return false;
            }
        }

        // make new student object
        StudentInfo newStudent;
        newStudent.studentName = studentName;
        newStudent.ufid = ufid;
        newStudent.residenceLocation = residenceLocation;

        // add all classes to schedule
        for (const string& code : classCodes) {
            newStudent.schedule.insert(code);
        }

        // if duplicates got passed in, schedule size wont match
        if (newStudent.schedule.size() != classCodes.size()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // store student record
        studentRecords[ufid] = newStudent;

        cout << "successful" << endl;
        return true;
    }

    // handle remove command
    if (command.rfind("remove ", 0) == 0) {
        stringstream lineStream(command);
        string action;
        string ufid;
        string extra;

        // read command word and ufid
        if (!(lineStream >> action >> ufid)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // remove should only have one argument after command word
        if (lineStream >> extra) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // ufid format has to be valid
        if (!ValidUFID(ufid)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // student has to exist
        auto it = studentRecords.find(ufid);
        if (it == studentRecords.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // erase student record
        studentRecords.erase(it);

        cout << "successful" << endl;
        return true;
    }

    // dropClass command
    if (command.rfind("dropClass ", 0) == 0) {
        stringstream lineStream(command);
        string action;
        string ufid;
        string classCode;
        string extra;

        // read command word, ufid, and class code
        if (!(lineStream >> action >> ufid >> classCode)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // dropClass should only have two arguments
        if (lineStream >> extra) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // ufid format validity
        if (!ValidUFID(ufid)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // class code format has to be valid and exist
        if (!ValidClassCode(classCode) || classes.find(classCode) == classes.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // student has to exist
        auto studentIt = studentRecords.find(ufid);
        if (studentIt == studentRecords.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // student has to currently have that class
        auto classIt = studentIt->second.schedule.find(classCode);
        if (classIt == studentIt->second.schedule.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // remove class from students schedule
        studentIt->second.schedule.erase(classIt);

        // if student now has zero classes, remove student too
        if (studentIt->second.schedule.empty()) {
            studentRecords.erase(studentIt);
        }

        cout << "successful" << endl;
        return true;
    }

    // invalid command rn
    cout << "unsuccessful" << endl;
    return false;
}