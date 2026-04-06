#include "CampusCompass.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <queue>
#include <unordered_set>

using namespace std;

CampusCompass::CampusCompass() {

}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    // clear old data in case this gets called again
    campusGraph.clear();
    classes.clear();
    studentRecords.clear();

    // try loading both csv files
    bool loadedEdges = LoadEdgeData(edges_filepath);
    bool loadedCourses = LoadCourseData(classes_filepath);

    // if either fails return false
    if (!loadedEdges || !loadedCourses) {
        return false;
    }

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

        int location1;
        int location2;
        int travelTime;

        // help for any invalid fields
        try {
            location1 = stoi(location1Str);
            location2 = stoi(location2Str);
            travelTime = stoi(travelTimeStr);
        }

        // dont crash, just skip
        catch(...) {
            continue;
        }

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

        int classLocation;

        // incase of invalid fields just skipp
        try {
            classLocation = stoi(classLocationStr);
        }

        catch(...) {
            continue;
        }

        // make a course object for current row
        CourseInfo currentCourse;
        currentCourse.classLocation = classLocation;
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
    return count >= 1 && count <= 6;
}

bool CampusCompass::ParseInsertCommand(const string& command, string& studentName, string& ufid, int& residenceLocation,
                                       vector<string>& classCodes) const {
    classCodes.clear();
    studentName.clear();
    ufid.clear();

    // insert must begin w insert
    if (command.rfind("insert ", 0) != 0) {
        return false;
    }

    // find quoted name
    size_t firstQuote = command.find('"');
    if (firstQuote == string::npos) {
        return false;
    }

    size_t secondQuote = command.find('"', firstQuote + 1);
    if (secondQuote == string::npos || secondQuote <= firstQuote) {
        return false;
    }

    // no extra quotes after quoted name
    if (command.find('"', secondQuote + 1) != string::npos) {
        return false;
    }

    // pull name
    studentName = command.substr(firstQuote + 1, secondQuote - firstQuote - 1);

    // everything after name
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

bool CampusCompass::ToggleClosure(int locationA, int locationB) {
    bool foundForward = false;
    bool foundBackward = false;

    // find edge a to b then toggle
    auto graphItA = campusGraph.find(locationA);
    if (graphItA != campusGraph.end()) {
        for (auto& edge : graphItA->second) {
            if (edge.adjacentId == locationB) {
                edge.available = !edge.available;
                foundForward = true;
                break;
            }
        }
    }

    // find edge b to a then toggle
    auto graphItB = campusGraph.find(locationB);
    if (graphItB != campusGraph.end()) {
        for (auto& edge : graphItB->second) {
            if (edge.adjacentId == locationA) {
                edge.available = !edge.available;
                foundBackward = true;
                break;
            }
        }
    }

    return foundForward && foundBackward;
}

string CampusCompass::EdgeStatus(int locationA, int locationB) const {
    auto graphIt = campusGraph.find(locationA);

    // if start location dne, edge dne
    if (graphIt == campusGraph.end()) {
        return "DNE";
    }

    // search edge a to b
    for (const auto& edge : graphIt->second) {
        if (edge.adjacentId == locationB) {
            if (edge.available) {
                return "open";
            }
            return "closed";
        }
    }
    return "DNE";
}

bool CampusCompass::IsConnected(int startLocation, int endLocation) const {
    // if same location = reachable
    if (startLocation == endLocation) {
        return true;
    }

    // start / end should exist in graph
    if (campusGraph.find(startLocation) == campusGraph.end() ||
        campusGraph.find(endLocation) == campusGraph.end()) {
        return false;
    }

    queue<int> toVisit;
    unordered_set<int> visited;

    toVisit.push(startLocation);
    visited.insert(startLocation);

    while (!toVisit.empty()) {
        int current = toVisit.front();
        toVisit.pop();

        auto graphIt = campusGraph.find(current);
        if (graphIt == campusGraph.end()) {
            continue;
        }

        for (const auto& edge : graphIt->second) {
            // ignore closed edges
            if (!edge.available) {
                continue;
            }

            int nextLocation = edge.adjacentId;

            if (nextLocation == endLocation) {
                return true;
            }

            if (visited.find(nextLocation) == visited.end()) {
                visited.insert(nextLocation);
                toVisit.push(nextLocation);
            }
        }
    }

    return false;
}

bool CampusCompass::ParseCommand(const string &command) {
    if (command.empty()) {
        cout << "unsuccessful" << endl;
        return false;
    }

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

        // store student
        studentRecords[ufid] = newStudent;

        cout << "successful" << endl;
        return true;
    }

    // handle remove
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

    // dropClass
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

        // dropClass should have two arguments
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

        // student has to currently have class
        auto classIt = studentIt->second.schedule.find(classCode);
        if (classIt == studentIt->second.schedule.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // remove class from schedule
        studentIt->second.schedule.erase(classIt);

        // if student has zero classes, remove student
        if (studentIt->second.schedule.empty()) {
            studentRecords.erase(studentIt);
        }

        cout << "successful" << endl;
        return true;
    }

    // replaceClass
    if (command.rfind("replaceClass ", 0) == 0) {
        stringstream lineStream(command);
        string action;
        string ufid;
        string oldClassCode;
        string newClassCode;
        string extra;

        // read command word, ufid, old class, and new class
        if (!(lineStream >> action >> ufid >> oldClassCode >> newClassCode)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // replaceClass should have three arguments
        if (lineStream >> extra) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // ufid format has to be valid
        if (!ValidUFID(ufid)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // old class code should at least have valid format
        if (!ValidClassCode(oldClassCode)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // new class code has to be valid and exist
        if (!ValidClassCode(newClassCode) || classes.find(newClassCode) == classes.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // student has to exist
        auto studentIt = studentRecords.find(ufid);
        if (studentIt == studentRecords.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // student has to actually have the old class first
        if (studentIt->second.schedule.find(oldClassCode) == studentIt->second.schedule.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // cant replace with a class they already have
        if (studentIt->second.schedule.find(newClassCode) != studentIt->second.schedule.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // swap old class for new class
        studentIt->second.schedule.erase(oldClassCode);
        studentIt->second.schedule.insert(newClassCode);

        cout << "successful" << endl;
        return true;
    }

    // removeClass command
    if (command.rfind("removeClass ", 0) == 0) {
        stringstream lineStream(command);
        string action;
        string classCode;
        string extra;

        // read command word and class code
        if (!(lineStream >> action >> classCode)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // removeClass should have one argument
        if (lineStream >> extra) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // class code format has to be valid
        if (!ValidClassCode(classCode)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // class has to exist
        if (classes.find(classCode) == classes.end()) {
            cout << "unsuccessful" << endl;
            return false;
        }

        int affectedStudents = 0;
        vector<string> studentsToRemove;

        // go through every student and remove class if they have
        for (auto& studentPair : studentRecords) {
            StudentInfo& currentStudent = studentPair.second;

            auto classIt = currentStudent.schedule.find(classCode);
            if (classIt != currentStudent.schedule.end()) {
                currentStudent.schedule.erase(classIt);
                affectedStudents++;

                // if student has zero classes remove after loop
                if (currentStudent.schedule.empty()) {
                    studentsToRemove.push_back(studentPair.first);
                }
            }
        }

        // if nobody had class then fail
        if (affectedStudents == 0) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // remove students with no classes
        for (const string& ufid : studentsToRemove) {
            studentRecords.erase(ufid);
        }

        cout << affectedStudents << endl;
        return true;
    }

    // toggleEdgesClosure
    if (command.rfind("toggleEdgesClosure ", 0) == 0) {
        stringstream lineStream(command);
        string action;
        int edgeCount;

        //read word and # of edge to toggle
        if (!(lineStream >> action >> edgeCount)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // need positive edge count
        if (edgeCount <= 0) {
            cout << "unsuccessful" << endl;
            return false;
        }

        vector<pair<int, int>> edgesToToggle;

        // read pair of location ids for edgeCount
        for (int i = 0; i < edgeCount; i++) {
            int locationA;
            int locationB;

            if (!(lineStream >> locationA >> locationB)) {
                cout << "unsuccessful" << endl;
                return false;
            }

            edgesToToggle.push_back({locationA, locationB});
        }
        
        // no extra input after expected
        string extra;
        if (lineStream >> extra) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // toggle listed edges
        for (const auto& edgePair : edgesToToggle) {
            ToggleClosure(edgePair.first, edgePair.second);
        }

        cout << "successful" << endl;
        return true;
    }

    // checkEdgeStatus
    if (command.rfind("checkEdgeStatus ", 0) == 0) {
        stringstream lineStream(command);
        string action;
        int locationA;
        int locationB;
        string extra;

        // read location ids and command
        if (!(lineStream >> action >> locationA >> locationB)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // only two ids after command
        if (lineStream >> extra) {
            cout << "unsuccessful" << endl;
            return false;
        }

        cout << EdgeStatus(locationA, locationB) << endl;
        return true;
    }

    // isConnected
    if (command.rfind("isConnected ", 0) == 0) {
        stringstream lineStream(command);
        string action;
        int locationA;
        int locationB;
        string extra;

        // read command and two location ids
        if (!(lineStream >> action >> locationA >> locationB)) {
            cout << "unsuccessful" << endl;
            return false;
        }

        // should not have extra input
        if (lineStream >> extra) {
            cout << "unsuccessful" << endl;
            return false;
        }

        if (IsConnected(locationA, locationB)) {
            cout << "successful" << endl;
        }
        else {
            cout << "unsuccessful" << endl;
        }

        return true;
    }

    cout << "unsuccessful" << endl;
    return false;
}