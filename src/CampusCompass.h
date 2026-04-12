#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <tuple>

using namespace std;

// one connection from location to adjacent location
struct CampusEdge {
    int adjacentId;
    int travelTime;
    bool available;
};

// store info abt course from classes.csv
struct CourseInfo {
    int classLocation;
    string startTime;
    string endTime;
};

// store info associated with student
struct StudentInfo {
    string studentName;
    string ufid;
    int residenceLocation;
    set<string> schedule;
};

class CampusCompass {
private:
    // main campus graph:
    // key = location id
    // value = list of adjacent edges
    unordered_map<int, vector<CampusEdge>> campusGraph;

    // map each class code to its associated course info
    unordered_map<string, CourseInfo> classes;

    // map each ufid to students info
    unordered_map<string, StudentInfo> studentRecords;

    // load edge data from edges.csv into graph
    bool LoadEdgeData(const string& filename);

    // load course data from classes.csv
    bool LoadCourseData(const string& filename);

    // print counts to make sure csv loading worked
    void PrintLoadSummary() const;

    // make sure ufid is 8 digits
    bool ValidUFID(const string& ufid) const;

    // make sure student name only has letters / spaces
    bool ValidName(const string& name) const;

    // make sure class code format is valid
    bool ValidClassCode(const string& code) const;

    // make sure class count is between 1 and 6
    bool ValidClassCount(int count) const;

    // parse insert command separately bc name is in quotes
    bool ParseInsertCommand(const string& command, string& studentName, string& ufid, int& residenceLocation,
                            vector<string>& classCodes) const;

    // remove entire student record by ufid
    bool RemoveStudentRecord(const string& ufid);

    // drop class from student schedule
    bool DropStudentClass(const string& ufid, const string& classCode);

    // replace a class in a students schedule with another
    bool SwapStudentClass(const string& ufid, const string& oldCode,
                          const string& newCode);

    // remove class from every student who has it
    int RemoveClassEverywhere(const string& classCode);

    // toggle edge between open / closed
    bool ToggleClosure(int locationA, int locationB);

    // return if an edge is open, closed, or dne
    string EdgeStatus(int locationA, int locationB) const;

    // check if one location can reach another with available edges
    bool IsConnected(int startLocation, int endLocation) const;

    // find shortest route only using open edges
    bool DijkstraShortestPath(int startLocation, int endLocation, int& shortestDistance, vector<int>& shortestPath) const;

    // shortest distance from home to class for student
    void PrintShortestEdges(const string& ufid) const;

    // print student zone cost
    void PrintStudentZone(const string& ufid) const;

    // find shortest path and total travel time between two locations
    pair<int, vector<int>> Dijkstra(int startLocation, int endLocation) const;

    // collect all vertices used in shortest paths from home to class
    set<int> CollectZoneVertices(const StudentInfo& student) const;

    // build subgraph used for student zone calc
    vector<tuple<int, int, int>> BuildZoneSubgraph(const set<int>& vertices) const;

    // get mst cost of the student zone subgraph
    int ComputeMSTCost(const set<int>& vertices, const vector<tuple<int, int, int>>& edges) const;

    // get final zone cost for one student
    int GetStudentZoneCost(const StudentInfo& student) const;

    // parse connect using current open edges
    bool IsConnectedCommand(int startLocation, int endLocation) const;

public:
    CampusCompass();
    bool ParseCSV(const string& edges_filepath, const string& classes_filepath);
    bool ParseCommand(const string& command);
};