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
#include "MatFinderOptions.h"

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
        {"playfor", required_argument, 0, 'o'},
        {"path", required_argument, 0, 'p'},
        {"moves", required_argument, 0, 'm'},
        {"lines", required_argument, 0, 'l'},
        {"hashmap_size", required_argument, 0, 't'},
        {"pf_movetime", required_argument, 0, 'f'},
        {"pa_movetime", required_argument, 0, 'a'},
        {"cp_treshold", required_argument, 0, 'c'},
    };

    int c;
    /* getopt_long stores the option index here. */
    int option_index = 0;

    while ((c = getopt_long(argc, argv,
                "hv:s:e:o:p:m:l:t:c:f:a:",
                long_options, &option_index)) != -1) {
        list<string> moveList;
        Board::Side playFor;
        switch (c) {

            case 'v':
                try {
                    MatFinderOptions::setVerboseLevel(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing verbose level");
                }
                break;

            case 's':
                MatFinderOptions::setStartingPos(optarg);
                break;

            case 'e':
                MatFinderOptions::setEngine(optarg);
                break;

            case 'p':
                MatFinderOptions::setPath(optarg);
                break;

            case 'm':
                moveList.clear();
                if (Utils::parseMovelist(moveList, optarg))
                    Utils::handleError("Error parsing movelist");
                MatFinderOptions::setUserMoves(moveList);
                break;

            case 'o':
                playFor = Board::getSideFromString(optarg);
                MatFinderOptions::setPlayFor(playFor);
                break;

            case 'l':
                try {
                    MatFinderOptions::setMaxLines(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing lines option");
                }
                break;

            case 't':
                try {
                    MatFinderOptions::setHashmapSize(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing hasmap size");
                }
                break;

            case 'c':
                try {
                    MatFinderOptions::setCpTreshold(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing centipawn treshold");
                }
                break;

            case 'f':
                try {
                    MatFinderOptions::setPlayforMovetime(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing playfor movetime");
                }
                break;

            case 'a':
                try {
                    MatFinderOptions::setPlayagainstMovetime(stoi(optarg));
                } catch (...) {
                    Utils::handleError("Error parsing playagainst movetime");
                }
                break;

            case 'h':
                Utils::output(Utils::helpMessage());
                exit(EXIT_SUCCESS);
                break;
            case '?':
                /* getopt_long already printed an error message. */
                exit(EXIT_FAILURE);

            default:
                abort ();
        }
    }

}


int main(int argc, char **argv)
{
    parseArgs(argc, argv);
    Utils::output(MatFinderOptions::getPretty(), 1);

    // Child error signal install
    struct sigaction action;
    action.sa_handler = Utils::handleError;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1, &action, NULL) < 0) {
        Utils::handleError("SIGUSR1 install error");
    }

    //The main object
    MatFinder *theFinder = new MatFinder();

    pid_t pid;
    // Create child process; both processes continue from here
    pid = fork();

    if (pid == pid_t(0))
    {
        theFinder->runEngine();

        // We should never reach this point
        // Tell the parent the exec failed
        kill(getppid(), SIGUSR1);
        exit(EXIT_FAILURE);

    } else if (pid > pid_t(0)) {

        //Run the finder
        int rt = theFinder->runFinder();

        Utils::handleError("runfinder()", rt);

        //Kill the child to exit properly
        kill(pid, SIGTERM);
    } else {
        Utils::handleError("Error: fork failed");
    }

    Utils::output("Deleting Finder", 5);
    delete theFinder;
    return 0;
}
