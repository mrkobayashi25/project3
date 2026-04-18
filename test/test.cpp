/*
#include <catch/catch_amalgamated.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <limits>
#include "CampusCompass.h"

using namespace std;

// runs full input through CampusCompass and captures output
static string RunScenario(const string& input) {
    CampusCompass compass;

    bool loaded = compass.ParseCSV("data/edges.csv", "data/classes.csv");
    REQUIRE(loaded);

    istringstream in(input);

    int no_of_lines = 0;
    in >> no_of_lines;
    in.ignore(numeric_limits<streamsize>::max(), '\n');

    ostringstream capturedOutput;
    streambuf* oldCoutBuffer = cout.rdbuf(capturedOutput.rdbuf());

    string command;
    for (int i = 0; i < no_of_lines; i++) {
        getline(in, command);
        compass.ParseCommand(command);
    }

    cout.rdbuf(oldCoutBuffer);
    return capturedOutput.str();
}

// invalid UFID test
TEST_CASE("Invalid UFID length prints unsuccessful", "[incorrect][insert]") {
    string input = R"(1
insert "Mason Downs" 1234567 20 2 CDA3101 COP3530
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// invalid class code test
TEST_CASE("Invalid class code format prints unsuccessful", "[incorrect][insert]") {
    string input = R"(1
insert "Mason Downs" 12345678 20 2 cda3101 COP3530
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// malformed insert test
TEST_CASE("Malformed insert missing quotes prints unsuccessful", "[incorrect][insert]") {
    string input = R"(1
insert Mason Downs 12345678 20 2 CDA3101 COP3530
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// extra argument test
TEST_CASE("Insert with extra arguments prints unsuccessful", "[incorrect][insert]") {
    string input = R"(1
insert "Mason Downs" 12345678 20 2 CDA3101 COP3530 EXTRA
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// duplicate student test
TEST_CASE("Duplicate student insert prints unsuccessful on second insert", "[incorrect][insert]") {
    string input = R"(2
insert "Mason Downs" 12345678 20 2 CDA3101 COP3530
insert "Mason Downs" 12345678 20 2 CDA3101 COP3530
)";
    string expectedOutput = R"(successful
unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// wrong class count test
TEST_CASE("Insert with wrong class count prints unsuccessful", "[incorrect][insert]") {
    string input = R"(1
insert "Mason Downs" 12345678 20 3 CDA3101 COP3530
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// duplicate class edge case
TEST_CASE("Insert with duplicate classes is unsuccessful", "[edge][insert]") {
    string input = R"(1
insert "Mason Downs" 12345678 20 2 CDA3101 CDA3101
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// minimal valid insert
TEST_CASE("Minimal valid schedule inserts successfully", "[edge][insert]") {
    string input = R"(1
insert "Mason Downs" 12345678 20 1 CDA3101
)";
    string expectedOutput = R"(successful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// dropping last class removes student
TEST_CASE("Dropping last class removes student", "[edge][dropClass][remove]") {
    string input = R"(3
insert "Mason Downs" 12345678 20 1 CDA3101
dropClass 12345678 CDA3101
remove 12345678
)";
    string expectedOutput = R"(successful
successful
unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// remove success test
TEST_CASE("remove removes an existing student successfully", "[remove]") {
    string input = R"(2
insert "Mason Downs" 12345678 20 1 CDA3101
remove 12345678
)";
    string expectedOutput = R"(successful
successful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// remove fail test
TEST_CASE("remove prints unsuccessful for missing student", "[remove][incorrect]") {
    string input = R"(1
remove 12345678
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// dropClass keep student if class remains
TEST_CASE("dropClass removes one class but keeps student if classes remain", "[dropClass]") {
    string input = R"(3
insert "Mason Downs" 12345678 20 2 CDA3101 COP3530
dropClass 12345678 CDA3101
remove 12345678
)";
    string expectedOutput = R"(successful
successful
successful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// dropClass missing student
TEST_CASE("dropClass prints unsuccessful when student does not exist", "[dropClass][incorrect]") {
    string input = R"(1
dropClass 12345678 CDA3101
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// dropClass missing class
TEST_CASE("dropClass prints unsuccessful when student does not have the class", "[dropClass][incorrect]") {
    string input = R"(2
insert "Mason Downs" 12345678 20 1 CDA3101
dropClass 12345678 COP3530
)";
    string expectedOutput = R"(successful
unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// replaceClass success
TEST_CASE("replaceClass successfully swaps classes", "[replaceClass]") {
    string input = R"(3
insert "Mason Downs" 12345678 20 1 CDA3101
replaceClass 12345678 CDA3101 COP3530
removeClass COP3530
)";
    string expectedOutput = R"(successful
successful
1
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// replaceClass missing original
TEST_CASE("replaceClass prints unsuccessful if original class not present", "[replaceClass][incorrect]") {
    string input = R"(2
insert "Mason Downs" 12345678 20 1 CDA3101
replaceClass 12345678 COP3530 MAC2311
)";
    string expectedOutput = R"(successful
unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// replaceClass duplicate new class
TEST_CASE("replaceClass prints unsuccessful if replacement class already exists", "[replaceClass][incorrect]") {
    string input = R"(2
insert "Mason Downs" 12345678 20 2 CDA3101 COP3530
replaceClass 12345678 CDA3101 COP3530
)";
    string expectedOutput = R"(successful
unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// removeClass working
TEST_CASE("removeClass removes class and prints count", "[removeClass]") {
    string input = R"(4
insert "Student A" 10000001 20 2 COP3530 MAC2311
insert "Student B" 10000002 21 1 COP3530
insert "Student C" 10000003 22 1 CDA3101
removeClass COP3530
)";
    string expectedOutput = R"(successful
successful
successful
2
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// removeClass invalid class
TEST_CASE("removeClass prints unsuccessful if class invalid", "[removeClass][incorrect]") {
    string input = R"(1
removeClass ABC1234
)";
    string expectedOutput = R"(unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// removeClass no students
TEST_CASE("removeClass prints unsuccessful if no students have class", "[removeClass][incorrect]") {
    string input = R"(2
insert "Mason Downs" 12345678 20 1 CDA3101
removeClass COP3530
)";
    string expectedOutput = R"(successful
unsuccessful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// edge status test
TEST_CASE("checkEdgeStatus returns open closed or DNE", "[edges][checkEdgeStatus]") {
    string input = R"(4
checkEdgeStatus 12 13
toggleEdgesClosure 1 12 13
checkEdgeStatus 12 13
checkEdgeStatus 999 1000
)";
    string expectedOutput = R"(open
successful
closed
DNE
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// same node connectivity
TEST_CASE("isConnected works for same node", "[edges][isConnected]") {
    string input = R"(1
isConnected 20 20
)";
    string expectedOutput = R"(successful
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// required dynamic shortest path test
TEST_CASE("printShortestEdges reachable then unreachable", "[printShortestEdges][required]") {
    string input = R"(4
insert "Brian" 35459999 20 1 PHY2048
printShortestEdges 35459999
toggleEdgesClosure 1 49 56
printShortestEdges 35459999
)";
    string expectedOutput = R"(successful
Time For Shortest Edges: Brian
PHY2048: 14
successful
Time For Shortest Edges: Brian
PHY2048: -1
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}

// student zone test
TEST_CASE("printStudentZone prints correctly", "[printStudentZone]") {
    string input = R"(2
insert "Mason Downs" 12345678 20 2 CDA3101 COP3530
printStudentZone 12345678
)";
    string expectedOutput = R"(successful
Student Zone Cost For Mason Downs: 20
)";
    REQUIRE(RunScenario(input) == expectedOutput);
}
    */