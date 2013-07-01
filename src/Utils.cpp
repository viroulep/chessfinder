/*
 * Matfinder, a program to help chess engines to find mat
 *
 * Copyright© 2013 Philippe Virouleau
 *
 * You can contact me at firstname.lastname@imag.fr
 * (Replace "firstname" and "lastname" with my actual names)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <sstream>
#include <fstream>
#include "Utils.h"
#include "MatFinderOptions.h"
#include "Line.h"
#include "Board.h"

using namespace std;

//TODO: exit program correctly on error

void Utils::signalHandler(int rc)
{
    handleError("Engine execution failed : " + to_string(rc));
}

void Utils::handleError(int rc)
{
    handleError("unspecified caller", rc);
}

void Utils::handleError(const string &msg)
{
    cerr << msg << endl;
    exit(EXIT_FAILURE);
}

void Utils::handleError(const string &caller, int rc)
{
    if (rc) {
        handleError("Error on : " + caller + ", rc=" + to_string(rc));
    }
}

void Utils::handleError(const string &msg, int rc,
        const string &fileName, int lineNumber)
{
    if (rc)
        cerr << fileName << ":" << lineNumber << endl;
    handleError(msg, rc);
}


void Utils::output(const string &msg, int level/* = 0*/)
{
    if (level <= MatFinderOptions::getVerboseLevel())
        cout << msg;
}

string Utils::helpMessage()
{
    ostringstream oss;
    oss << "MatFinder, a program to help engines to find mat\n";
    oss << "\n";
    oss << "Options\n";
    oss << "\n";
    oss << "\t" << "--startpos=fenstring, -s fen_string\n";
    oss << "\t\t" << "Defines the initial board state, fen format.\n"
        << "\t\t" << "Example value : "
        << "\"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"\n"
        << "\t\t" << "Default value : \"" << DEFAULT_STARTPOS << "\"\n";
    oss << "\n";
    oss << "\t" << "--moves=moves, -s moves\n";
    oss << "\t\t" << "Defines the main line after the starting position.\n"
        << "\t\t" << "Example value : "
        << "\"e2e4 c7d5\"\n"
        << "\t\t" << "Default value : \"<none>\"\n";
    oss << "\n";
    oss << "\t" << "--position_file=pathtofile, -i pathtofile\n";
    oss << "\t\t" << "Specify a file to load the positions from. If this\n"\
        "\t\t  options is given, --startpos and --moves will be ignored.\n"\
        "\t\t  All the positions in this file are to be processed.\n"
        << "\t\t" << "There must be one position per line, following this "\
        "format : \n"
        << "\t\t\"position (startpos|fen thefenstring) "\
        "[moves additionnalmoves]\"\n";
    oss << "\n";
    oss << "\t" << "--engine=engine, -e engine\n";
    oss << "\t\t" << "Defines the uci engine command.\n"
        << "\t\t" << "Default value : \"" << DEFAULT_ENGINE << "\"\n";
    oss << "\n";
    oss << "\t" << "--playfor=side, -o side\n";
    oss << "\t\t" << "Defines the side the engine plays for.\n"
        << "\t\t" << "Default value : \""
        << Board::to_string(Board::Side::DEFAULT_PLAY_FOR) << "\"\n";
    oss << "\n";
    oss << "\t" << "--path=path, -p path\n";
    oss << "\t\t" << "Defines the engine binary's path.\n"
        << "\t\t" << "Default value : \"" << DEFAULT_PATH << "\"\n";
    oss << "\n";
    oss << "\t" << "--pf_movetime=msec, -f msec\n";
    oss << "\t\t" << "Playfor Movetime. Defines the engine thinking time,\n"
        << "\t\t\twhen playing on the side he \"plays for\".\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_PLAYFOR_MOVETIME_) << "\n";
    oss << "\n";
    oss << "\t" << "--pa_movetime=msec, -a msec\n";
    oss << "\t\t" << "Playagainst Movetime. Defines the engine thinking time,\n"
        << "\t\t\twhen he answers the side he \"plays for\".\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_PLAYAGAINST_MOVETIME_) << "\n";
    oss << "\n";
    oss << "\t" << "--lines=n, -l n\n";
    oss << "\t\t" << "Defines the number of lines to evaluate"
        << " (MultiPV uci's option)\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_MAX_LINES) << "\n";
    oss << "\n";
    oss << "\t" << "--verbose=level, -v level\n";
    oss << "\t\t" << "Defines the verbose level. 1 displays lines followed,\n"
        << "\t\t\t" << "2 add the chessboard, 3+ is for debug purpose\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_VERBOSE_LEVEL) << "\n";
    oss << "\n";
    oss << "\t" << "--hashmap_size=size, -t size\n";
    oss << "\t\t" << "Defines the engine Hashmap size in MBytes.\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_HASHMAP_SIZE) << "\n";
    oss << "\n";
    oss << "\t" << "--cp_treshold=limit, -t limit\n";
    oss << "\t\t" << "Centipawn treshold : defines the treshold value for\n"\
        "\t\t  an evaluation to be considered.\n"\
        "\t\tAll lines with an evaluation (in centipawn) below this value\n"\
        "\t\t  will be ignored.\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_HASHMAP_SIZE) << "\n";
    oss << "\n";
    oss << "Examples\n";
    oss << "\n";
    oss << "\tStart the finder on gardnerfish, studying 1. c4 opening,\n"
        << "\twith detail informations about lines played :\n";
    oss << "\t$ ./matfinder --verbose --moves=\"c3c4\" --engine=\"gardnerfish\"\n";
    oss << "\n";
    oss << "Contact\n";
    oss << "\tPhilippe Virouleau <philippe.viroulea@imag.fr>\n";
    return oss.str();
}

const string Utils::RED = "\e[31;1m";
const string Utils::RESET = "\e[0m";

string Utils::listToString(list<string> &theList)
{
    ostringstream oss;
    if (theList.empty())
        oss << "<none>";
    for (list<string>::iterator it = theList.begin(),
            itEnd = theList.end();
            it != itEnd; ++it) {
        if (it != theList.begin())
            oss << " ";
        oss << (*it);
    }
    return oss.str();
}

int Utils::parseMovelist(list<string> &theList, string moves)
{
    istringstream is(moves);
    Utils::output("Parsing moves : " + moves + "\n", 3);
    string mv;
    while (is >> skipws >> mv) {
        if (!Board::checkMove(mv))
            return 1;
        theList.push_back(mv);
    }
    return 0;
}

PositionList Utils::positionListFromFile(string fileName)
{
    string line;
    PositionList ret;
    ifstream inputFile(fileName);
    if (inputFile.is_open()) {
        while (getline(inputFile, line) && inputFile.good()) {
            pair<string, list<string>> toAdd;
            istringstream is(line);
            string token;
            is >> skipws >> token;
            if (token != "position")
                Utils::handleError("Position must start with \"position\"");
            is >> token;
            if (token == "startpos") {
                Utils::output("Startpos is the pos\n", 4);
                toAdd.first = token;
                //get "moves" token, if any
                is >> token;
            } else if (token == "fen") {
                Utils::output("Startpos is a fen\n", 4);
                string fenString;
                while (is >> token && token != "moves") {
                    fenString += token + " "; 
                }
                //NOTE: ugly way of getting rid off last whitespace
                if (fenString.size()) {
                    fenString = fenString.substr(0, fenString.size() - 1);
                    toAdd.first = fenString;
                } else {
                    Utils::handleError("fen is empty");
                }
            } else {

                Utils::handleError("Missing 'fen' keyword");
            }
            list<string> moves;
            if (is.good()) {
                //if we are here we need to parse moves
                if (token != "moves") {
                    if (token.size() > 0)
                        Utils::handleError("Unrecognize token \"" + token + "\"");
                } else {
                    Utils::output("Parsing moves\n");
                    string mv;
                    while (is >> skipws >> mv) {
                        if (!Board::checkMove(mv))
                            handleError("Error parsing move list");
                        moves.push_back(mv);
                    }
                }
            } else
                Utils::output("End of input\n");
            toAdd.second = moves;
            Utils::output("Adding position : \"" + toAdd.first + "\" with"\
                    " moves : \n", 3);
            Utils::output("\t" + Utils::listToString(toAdd.second) +"\n", 3);
            ret.push_back(toAdd);
        }
    } else
        Utils::handleError("Error opening file \"" + fileName + "\"");
    return ret;
}

void Utils::getTimeout(struct timespec *ts, int seconds)
{
    struct timeval tp;
    int rc = gettimeofday(&tp, NULL);
    Utils::handleError("gettimeofday()\n", rc);

    /* Convert from timeval to timespec */
    ts->tv_sec  = tp.tv_sec;
    ts->tv_nsec = tp.tv_usec * 1000;
    ts->tv_sec += seconds;
}
