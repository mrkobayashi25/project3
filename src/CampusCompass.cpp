#include "CampusCompass.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

CampusCompass::CampusCompass() {
    // ome back later
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

        // break current csv row into bits
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

    // count total stored edges in list
    for (const auto& location : campusGraph) {
        edgeCount += location.second.size();
    }

    // divide by 2 since each undirected edge was stored twice
    edgeCount /= 2;

    // temp debug output for csv loading
    cout << "locations loaded: " << campusGraph.size() << endl;
    cout << "edges loaded: " << edgeCount << endl;
    cout << "class codes loaded: " << classes.size() << endl;
}

bool CampusCompass::ParseCommand(const string &command) {
    // come back later
    bool is_valid = true;

    return is_valid;
}