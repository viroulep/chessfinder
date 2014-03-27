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
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <memory>
#include <list>
#include <string>
#include <getopt.h>

#include "Utils.h"
#include "Output.h"
#include "Options.h"
/*#include "Line.h"*/
#include "MatFinder.h"
#include "OracleFinder.h"
/*#include "MatfinderOptions.h"*/
/*#include "CommonMain.h"*/

using namespace std;


void parseArgs(int argc, char **argv)
{
    //Program options
    const static struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"verbose", required_argument, 0, 'v'},
        {"startpos", required_argument, 0, 's'},
        {"engine", required_argument, 0, 'e'},
        {"path", required_argument, 0, 'p'},
        {"moves", required_argument, 0, 'm'},
        {"lines", required_argument, 0, 'l'},
        {"position_file", required_argument, 0, 'i'},
        {"hashmap_size", required_argument, 0, 't'},
        {"pf_movetime", required_argument, 0, 'f'},
        {"pa_movetime", required_argument, 0, 'a'},
        {"cp_treshold", required_argument, 0, 'c'},
        {"output_file", required_argument, 0, 'o'},
        {"input_file", required_argument, 0, 'n'},
        {"mateq", required_argument, 0, 'w'},
        {"engine_threads", required_argument, 0, 'x'},
        {"comparator", required_argument, 0, 'u'},
    };

    int c;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    string startingPos;
    list<string> moveList;
    PositionList posList;

    while ((c = getopt_long(argc, argv,
                "hi:v:s:e:p:m:l:t:c:f:a:o:u:n:w:x:",
                long_options, &option_index)) != -1) {
        switch (c) {

            case 'v':
                try {
                    /*MatfinderOptions::setVerboseLevel(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing verbose level");
                }
                break;

            case 's':
                startingPos = optarg;
                break;

            case 'e':
                /*MatfinderOptions::setEngine(optarg);*/
                break;

            case 'i':
                /*posList = Utils::positionListFromFile(optarg);*/
                /*MatfinderOptions::setPositionList(posList);*/
                break;

            case 'p':
                /*MatfinderOptions::setPath(optarg);*/
                break;

            case 'm':
                /*
                 *moveList.clear();
                 *if (Utils::parseMovelist(moveList, optarg))
                 *    Err::handle("Error parsing movelist");
                 */
                //Adding the move is done after this loop
                //MatfinderOptions::setUserMoves(moveList);
                break;

            case 'l':
                try {
                    /*MatfinderOptions::setMaxLines(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing lines option");
                }
                break;

            case 't':
                try {
                    /*MatfinderOptions::setHashmapSize(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing hasmap size");
                }
                break;

            case 'c':
                try {
                    /*MatfinderOptions::setCpTreshold(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing centipawn treshold");
                }
                break;

            case 'f':
                try {
                    /*MatfinderOptions::setPlayforMovetime(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing playfor movetime");
                }
                break;

            case 'a':
                try {
                    /*MatfinderOptions::setPlayagainstMovetime(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing playagainst movetime");
                }
                break;

            case 'h':
                Out::output(Utils::helpMessage());
                exit(EXIT_SUCCESS);
                break;

            case 'o':
                /*MatfinderOptions::setOutputFile(optarg);*/
                break;

            case 'u':
                /*MatfinderOptions::setMoveComparator(optarg);*/
                break;

            case 'n':
                /*MatfinderOptions::setInputFile(optarg);*/
                break;

            case 'w':
                try {
                    /*MatfinderOptions::setMateEquiv(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing mateq value");
                }
                break;

            case 'x':
                try {
                    /*MatfinderOptions::setEngineThreads(stoi(optarg));*/
                } catch (...) {
                    Err::handle("Error parsing engine threads");
                }
                break;

            case '?':
                /* getopt_long already printed an error message. */
                exit(EXIT_FAILURE);

            default:
                abort ();
        }
    }
    if (startingPos.size() == 0 && moveList.size() > 0)
        startingPos = "startpos";

    if (startingPos.size() > 0)
        Options::getInstance().addPositionToList(startingPos, moveList);

}

int main(int argc, char **argv)
{
    parseArgs(argc, argv);


    /*Out::output(Options::getInstance().getPretty(), 1);*/

    // Child error signal install
    struct sigaction action;
    action.sa_handler = Err::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1, &action, NULL) < 0) {
        Err::handle("SIGUSR1 install error");
    }

    Config defconf;
    Options &options = Options::getInstance();
    options.addConfig(defconf);

    /*Setup some engine options*/
    Comm::EngineOptions engine_options;
    engine_options.insert(make_pair("MultiPV", "254"));
    engine_options.insert(make_pair("UCI_AnalyseMode", "true"));

    Comm::UCICommunicatorPool &pool = Comm::UCICommunicatorPool::getInstance();

    /*Create the engine and its communicator*/
    int commId = pool.create<Comm::LocalUCICommunicator>(
                                   Options::getInstance().getEngineFullpath(),
                                   engine_options);

    //The main object
#ifdef MATFINDER
    MatFinder *theFinder = new MatFinder(commId);
#elif ORACLEFINDER
    OracleFinder *theFinder = new OracleFinder(commId);
#else
    Finder *theFinder = nullptr;
#endif

    //Run the finder
    if (theFinder) {
        int rt = theFinder->runFinder();

        Err::handle("runfinder()", rt);

        Out::output("Deleting Finder", 5);
        delete theFinder;
    }
    /*CommonMain::theMain(argc, argv, theFinder);*/
    return 0;
}
