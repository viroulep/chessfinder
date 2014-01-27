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

#include "Thread.h"
#include "Engine.h"
#include "Utils.h"
#include "Line.h"
#include "MatFinder.h"
#include "CommonMain.h"
#include "Options.h"

using namespace std;

void CommonMain::parseArgs(int argc, char **argv)
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
                    Options::setVerboseLevel(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing verbose level");
                }
                break;

            case 's':
                startingPos = optarg;
                break;

            case 'e':
                Options::setEngine(optarg);
                break;

            case 'i':
                posList = Utils::positionListFromFile(optarg);
                Options::setPositionList(posList);
                break;

            case 'p':
                Options::setPath(optarg);
                break;

            case 'm':
                moveList.clear();
                if (Utils::parseMovelist(moveList, optarg))
                    Utils::handleError("Error parsing movelist");
                //Adding the move is done after this loop
                //Options::setUserMoves(moveList);
                break;

            case 'l':
                try {
                    Options::setMaxLines(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing lines option");
                }
                break;

            case 't':
                try {
                    Options::setHashmapSize(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing hasmap size");
                }
                break;

            case 'c':
                try {
                    Options::setCpTreshold(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing centipawn treshold");
                }
                break;

            case 'f':
                try {
                    Options::setPlayforMovetime(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing playfor movetime");
                }
                break;

            case 'a':
                try {
                    Options::setPlayagainstMovetime(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing playagainst movetime");
                }
                break;

            case 'h':
                Utils::output(Utils::helpMessage());
                exit(EXIT_SUCCESS);
                break;

            case 'o':
                Options::setOutputFile(optarg);
                break;

            case 'u':
                Options::setMoveComparator(optarg);
                break;

            case 'n':
                Options::setInputFile(optarg);
                break;

            case 'w':
                try {
                    Options::setMateEquiv(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing mateq value");
                }
                break;

            case 'x':
                try {
                    Options::setEngineThreads(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing engine threads");
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
        Options::addPositionToList(startingPos, moveList);

}


int CommonMain::theMain(int argc, char **argv, Finder *theFinder)
{
    Utils::output(Options::getPretty(), 1);

    // Child error signal install
    struct sigaction action;
    action.sa_handler = Utils::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1, &action, NULL) < 0) {
        Utils::handleError("SIGUSR1 install error");
    }


    pid_t pid;
    // Create child process; both processes continue from here
    pid = fork();

    if (pid == pid_t(0))
    {
        theFinder->runEngine();

        // We should never reach this point
        // Tell the parent the exec failed
        kill(getppid(), SIGUSR1);
        Utils::handleError("Engine execution failed");
        exit(EXIT_FAILURE);

    } else if (pid > pid_t(0)) {

        //Run the finder
        int rt = theFinder->runFinder();

        Utils::handleError("runfinder()", rt);

        //Kill the child to exit properly
        kill(pid, SIGTERM);
        Options::setMoveComparator(nullptr);
    } else {
        Utils::handleError("Error: fork failed");
    }

    return 0;
}
