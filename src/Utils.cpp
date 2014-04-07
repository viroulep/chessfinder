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
#include "ChessboardTypes.h"
#include "Output.h"

using namespace std;

namespace Utils {
    string helpMessage()
    {
        ostringstream oss;
        oss << "MatFinder, a program to help engines to find mate\n";
        oss << "\n";
        oss << "Options\n";
        oss << "\n";
        oss << "    " << "--help, -h\n";
        oss << "        " << "Show this help message.\n";
        oss << "\n";
        oss << "    " << "--startpos=fenstring, -s fen_string\n";
        oss << "        " << "Add a position to run the finder on, fen format.\n"
            << "        " << "If you want to specify multiple positions, look at "
            << "the --position_file option.\n"
            << "        " << "Example value : "
            << "\"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"\n";
        oss << "\n";
        oss << "    " << "--moves=moves, -m moves\n";
        oss << "        " << "Defines the main line after the starting position.\n"
            << "        " << "Example value : "
            << "\"e2e4 c7d5\"\n";
        oss << "\n";
        oss << "    " << "--position_file=pathtofile, -f pathtofile\n";
        oss << "        " << "Specify a file to load the positions from.\n"\
            "        All the positions in this file are to be processed by "\
            "the finder.\n"
            << "        " << "There must be one position per line, following this "\
            "format : \n"
            << "        \"position fen theFenString "\
            "[moves additionnalmoves]\"\n";
        oss << "\n";
        oss << "    " << "--verbose=level, -v level\n";
        oss << "        " << "Defines the verbose level. 1 displays lines followed,\n"
            << "            " << "2 adds the chessboard, 3+ is for debug purpose\n";
        oss << "\n";
        oss << "    " << "--input_file=file, -i file\n";
        oss << "        " << "Defines the input file from which to load the table\n"\
            "         used by oraclefinder.\n";
        oss << "\n";
        oss << "    " << "--output_file=file, -o file\n";
        oss << "        " << "Defines the output file for the table generated by\n"\
            "         oraclefinder.\n";
        oss << "\n";
        oss << "    " << "--config_file=file, -c file\n";
        oss << "        " << "Gives an additional configuration file (See the "\
            "configuration section for informations and examples).\n";
        oss << "\n";
        oss << "Examples\n";
        oss << "\n";
        oss << "    Start the finder on the standard starting position, "\
            "studying 1. c4 opening,\n"
            << "    with detail informations about lines played :\n";
        oss << "    $ ./matfinder --verbose --moves=\"c3c4\"\n";
        oss << "\n";
        oss << "Configuration file\n";
        oss << "    Both matfinder and oraclefinder use configuration file to\n"\
            "    specify engine options and to set the multiple options of the finders.\n\n";
        oss << "    The default file read is the file \"chessfinderrc\" in current directory.\n";
        oss << "    The configuration file given in program's options overrides the"\
               "default files.\n";
        oss << "    It's split into four sections, describded below :\n";
        oss << "    - Engine\n";
        oss << "        path : the path to the engine (default is /usr/local/bin/stockfish)\n";
        oss << "        variant : the chess variant to use (default is standard)\n";
        oss << "                  Other possible values are : gardner, losalamos\n";
        oss << "        hashmap_size : size of engine's hashtable, in MB (default is 2048)\n";
        oss << "        threads : number of threads the engine should use\n";
        oss << "    - Finder\n";
        oss << "        verbose_level : the verbose level (default is 0)\n";
        oss << "        cutoff_treshold : define the value for the draw in centipawn\n";
        oss << "                          (Default is 100, which means lines between\n";
        oss << "                          -1.0 and 1.0 will be considered as draw)\n";
        oss << "        playfor_movetime : the time given to the side the finder plays for (ms)\n";
        oss << "                           (default is 1500)\n";
        oss << "        playagaint_movetime : the time given to the side the finder plays"\
               "against (ms)\n";
        oss << "                           (default is 1000)\n";
        oss << "    - Matfinder\n";
        oss << "        lines : number of lines to consider (default is 8)\n";
        oss << "        mate_treshold : minimal value for an evaluation to be considered"\
               "as mate.\n";
        oss << "                        (default is 10000 centipawn)\n";
        oss << "    - Oraclefinder\n";
        oss << "        comparator : the move comparator to use (default is \"map\")\n";
        oss << "                     Other values are \"default\".\n";
        oss << "\n";
        oss << "Contact\n";
        oss << "    Philippe Virouleau <philippe.viroulea@imag.fr>\n";
        oss << "    Mehdi Mhalla <mehdi.mhalla@imag.fr>\n";
        oss << "    Frederic Prost <frederic.prost@imag.fr>\n";
        return oss.str();
    }

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
                        Err::handle("parseMovelist",
                                    Board::parseMovelist(moves, is));
                    }
                } else
                    Out::output("End of input\n");
                toAdd.second = moves;
                Out::output("Adding position : \"" + toAdd.first + "\" with"\
                        " moves : \n", 3);
                Out::output("    " + Utils::listToString(toAdd.second) +"\n", 3);
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
