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
#include "Output.h"

using namespace std;

namespace Utils {
    string helpMessage()
    {
        ostringstream oss;
        oss << "MatFinder, a program to help engines to find mat\n";
        oss << "\n";
        oss << "Options\n";
        oss << "\n";
        oss << "\t" << "--startpos=fenstring, -s fen_string\n";
        oss << "\t\t" << "Defines the initial board state, fen format.\n"
            << "\t\t" << "Example value : "
            << "\"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"\n";
        /*<< "\t\t" << "Default value : \"" << DEFAULT_STARTPOS << "\"\n";*/
        oss << "\n";
        oss << "\t" << "--moves=moves, -m moves\n";
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
        oss << "\t\t" << "Defines the uci engine command.\n";
        /*<< "\t\t" << "Default value : \"" << DEFAULT_ENGINE << "\"\n";*/
        oss << "\n";
        oss << "\t" << "--path=path, -p path\n";
        oss << "\t\t" << "Defines the engine binary's path.\n";
        /*<< "\t\t" << "Default value : \"" << DEFAULT_PATH << "\"\n";*/
        oss << "\n";
        oss << "\t" << "--pf_movetime=msec, -f msec\n";
        oss << "\t\t" << "Playfor Movetime. Defines the engine thinking time,\n"
            << "\t\t\twhen playing on the side he \"plays for\".\n"
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_PLAYFOR_MOVETIME_) << "\n";*/
        oss << "\n";
        oss << "\t" << "--pa_movetime=msec, -a msec\n";
        oss << "\t\t" << "Playagainst Movetime. Defines the engine thinking time,\n"
            << "\t\t\twhen he answers the side he \"plays for\".\n"
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_PLAYAGAINST_MOVETIME_) << "\n";*/
        oss << "\n";
        oss << "\t" << "--lines=n, -l n\n";
        oss << "\t\t" << "Defines the number of lines to evaluate"
            << " (MultiPV uci's option)\n"
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_MAX_LINES) << "\n";*/
        oss << "\n";
        oss << "\t" << "--verbose=level, -v level\n";
        oss << "\t\t" << "Defines the verbose level. 1 displays lines followed,\n"
            << "\t\t\t" << "2 add the chessboard, 3+ is for debug purpose\n"
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_VERBOSE_LEVEL) << "\n";*/
        oss << "\n";
        oss << "\t" << "--hashmap_size=size, -t size\n";
        oss << "\t\t" << "Defines the engine Hashmap size in MBytes.\n"
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_HASHMAP_SIZE) << "\n";*/
        oss << "\n";
        oss << "\t" << "--cp_treshold=limit, -c limit\n";
        oss << "\t\t" << "Centipawn treshold : defines the treshold value for\n"\
            "\t\t  an evaluation to be considered.\n"\
            "\t\tAll lines with an evaluation (in centipawn) below this value\n"\
            "\t\t  will be ignored.\n"
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_TRESHOLD) << "\n";*/
        oss << "\t" << "--mateq=eval, -w mateeq\n";
        oss << "\t\t" << "Evaluation equivalent to mate.\n"\
            "\t\tAll lines with an evaluation (in centipawn) higher than this"\
            "value\n"\
            "\t\t  will be considered as mate .\n"
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_MATE_EQUIV) << "\n";*/
        oss << "\n";
        oss << "\t" << "--engine_threads=nthreads, -x nthreads\n";
        oss << "\t\t" << "Number of threads used by the engine.\n"\
            << "\t\t" << "Default value : ";
        /*<< to_string(DEFAULT_THREADS) << "\n";*/
        oss << "\n";
        oss << "\t" << "--input_file=file, -n file\n";
        oss << "\t\t" << "Defines the input file from which to load the table\n"\
            "\t\t used by oraclefinder.\n";
        oss << "\n";
        oss << "\t" << "--output_file=file, -o file\n";
        oss << "\t\t" << "Defines the output file for the table generated by\n"\
            "\t\t oraclefinder.\n";
        oss << "\n";
        oss << "\t" << "--comparator=comp, -u comp\n";
        oss << "\t\t" << "Defines the move comparator used by the oracle.\n";
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

    const string RED = "\e[31;1m";
    const string RESET = "\e[0m";

    string listToString(const list<string> &theList)
    {
        ostringstream oss;
        if (theList.empty())
            oss << "<none>";
        for (list<string>::const_iterator it = theList.begin(),
                itEnd = theList.end();
                it != itEnd; ++it) {
            if (it != theList.begin())
                oss << " ";
            oss << (*it);
        }
        return oss.str();
    }

    int parseMovelist(list<string> &theList, string moves)
    {
        istringstream is(moves);
        Out::output("Parsing moves : " + moves + "\n", 3);
        string mv;
        while (is >> skipws >> mv) {
            /*TODO restore the check*/
            /*if (!Board::checkMove(mv))*/
                /*return 1;*/
            theList.push_back(mv);
        }
        return 0;
    }

    PositionList positionListFromFile(string fileName)
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
                    Err::handle("Position must start with \"position\"");
                is >> token;
                if (token == "startpos") {
                    Out::output("Startpos is the pos\n", 4);
                    toAdd.first = token;
                    //get "moves" token, if any
                    is >> token;
                } else if (token == "fen") {
                    Out::output("Startpos is a fen\n", 4);
                    string fenString;
                    while (is >> token && token != "moves") {
                        fenString += token + " ";
                    }
                    //NOTE: ugly way of getting rid off last whitespace
                    if (fenString.size()) {
                        fenString = fenString.substr(0, fenString.size() - 1);
                        toAdd.first = fenString;
                    } else {
                        Err::handle("fen is empty");
                    }
                } else {
                    Err::handle("Missing 'fen' keyword");
                }

                list<string> moves;
                if (is.good()) {
                    //if we are here we need to parse moves
                    if (token != "moves") {
                        if (token.size() > 0)
                            Err::handle("Unrecognize token \"" + token + "\"");
                    } else {
                        Out::output("Parsing moves\n", 3);
                        string mv;
                        while (is >> skipws >> mv) {
                            /*TODO restore check*/
                            /*if (!Board::checkMove(mv))*/
                                /*handleError("Error parsing move list");*/
                            moves.push_back(mv);
                        }
                    }
                } else
                    Out::output("End of input\n");
                toAdd.second = moves;
                Out::output("Adding position : \"" + toAdd.first + "\" with"\
                        " moves : \n", 3);
                Out::output("\t" + Utils::listToString(toAdd.second) +"\n", 3);
                ret.push_back(toAdd);
            }
        } else
            Err::handle("Error opening file \"" + fileName + "\"");
        return ret;
    }

    void getTimeout(struct timespec *ts, int seconds)
    {
        struct timeval tp;
        int rc = gettimeofday(&tp, NULL);
        Err::handle("gettimeofday()\n", rc);

        /* Convert from timeval to timespec */
        ts->tv_sec  = tp.tv_sec;
        ts->tv_nsec = tp.tv_usec * 1000;
        ts->tv_sec += seconds;
    }
}
