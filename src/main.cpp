#include <iostream>
#include <limits>

#include "CampusCompass.h"

using namespace std;

int main() {
    // controller object for entire project
    // graph, students, classes, and command logic
    CampusCompass compass;

    // load needed csv data before commands are run per requirements
    compass.ParseCSV("data/edges.csv", "data/classes.csv");

    // first input line is number of commands that follow
    int no_of_lines;
    string command;

    cin >> no_of_lines;

    // only read int command #
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    // read specified amnt of full command lines
    for (int i = 0; i < no_of_lines; i++) {
        getline(cin, command);

        // send full command line to parser
        compass.ParseCommand(command);
    }

    return 0;
}