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
#include "ConfigParser.h"
#include "MatFinder.h"
#include "OracleFinder.h"

using namespace std;


void parseArgs(int argc, char **argv)
{
    //Program options
    const static struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"verbose", required_argument, 0, 'v'},
        {"startpos", required_argument, 0, 's'},
        {"moves", required_argument, 0, 'm'},
        {"position_file", required_argument, 0, 'f'},
        {"output_file", required_argument, 0, 'o'},
        {"input_file", required_argument, 0, 'i'},
        {"config_file", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int c;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    string startingPos;
    list<string> moveList;
    PositionList posList;
    Options &opt_instance = Options::getInstance();

    while ((c = getopt_long(argc, argv,
                /*"hi:v:s:e:p:m:l:t:c:f:a:o:u:n:w:x:",*/
                "hv:s:m:f:o:i:c:",
                long_options, &option_index)) != -1) {
        switch (c) {

            case 'v':
                try {
                    opt_instance.setVerboseLevel(stoi(optarg));
                } catch (...) {
                    Err::handle("Error parsing verbose level");
                }
                break;

            case 's':
                startingPos = optarg;
                break;

            case 'f':
                posList = Utils::positionListFromFile(optarg);
                opt_instance.setPositionList(posList);
                break;

            case 'm':
                moveList.clear();
                if (Board::parseMovelist(moveList, optarg))
                    Err::handle("Error parsing movelist");
                /*Adding the move is done after this loop*/
                break;

            case 'c':
                try {
                    Config user(optarg);
                    opt_instance.addConfig(user);
                } catch (...) {
                    Err::handle("Unable to load user-defined configuration file");
                }
                break;

            case 'h':
                Out::output(Utils::helpMessage());
                exit(EXIT_SUCCESS);
                break;

            case 'o':
                opt_instance.setOutputFile(optarg);
                break;

            case 'i':
                opt_instance.setInputFile(optarg);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                exit(EXIT_FAILURE);

            default:
                abort ();
        }
    }
    if (startingPos.size() > 0)
        Options::getInstance().addPositionToList(startingPos, moveList);

}

void sigintHandler(int rc)
{
    OracleFinder::dumpStat();
    Err::handle("Program interrupted by user (rc = " + std::to_string(rc) + ")");
}

int main(int argc, char **argv)
{
    try {
        Config defconf;
        Options &options = Options::getInstance();
        options.addConfig(defconf);
    } catch (...) {
        Out::output("No default configuration file found\n");
    }

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
    struct sigaction actInt;
    actInt.sa_handler = sigintHandler;
    sigemptyset(&actInt.sa_mask);
    actInt.sa_flags = 0;
    if (sigaction(SIGINT, &actInt, NULL) < 0) {
        Err::handle("SIGINT install error");
    }

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
