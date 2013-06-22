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
        {"verbose", optional_argument, 0, 'v'},
        {"startpos", required_argument, 0, 's'},
        {"engine", required_argument, 0, 'e'},
        {"playfor", required_argument, 0, 'o'},
        {"path", required_argument, 0, 'p'},
        {"moves", required_argument, 0, 'm'},
        {"lines", required_argument, 0, 'l'},
        {"hashmap_size", required_argument, 0, 't'},
        {"pf_movetime", required_argument, 0, 'f'},
        {"pa_movetime", required_argument, 0, 'a'},
    };

    int c;
    /* getopt_long stores the option index here. */
    int option_index = 0;

    while ((c = getopt_long(argc, argv,
                "hv::s:e:p:m:l:t:f:a:",
                long_options, &option_index)) != -1) {
        list<string> moveList;
        int value;
        side_t playFor = UNDEFINED;
        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'v':
                value = 1;
                //FIXME: -v n is not recognize properly
                try {
                    if (optarg)
                        value = stoi(optarg);
                } catch (...) {
                    Utils::handleError("Error parsing verbose level");
                }
                MatFinderOptions::setVerboseLevel(value);
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
                if (Utils::parseSide(&playFor, optarg))
                    Utils::handleError("Error parsing playfor side");
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
        perror("SIGUSR1 install error");
        exit(EXIT_FAILURE);
    }

    //TODO: delegate to matfinder
    //Engine gardner("losalamosfish", "/usr/local/bin");
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
        //TODO: take position and moves as param !
        cout << "Running finder on startpos, using losalamosfish\n";
        cout << "The aim is to prove that all unbalanced lines lead to black mating white.\n";
        //string testFenMat("8/8/1rq2k2/1p1p1p2/1b1p1B2/1P1P1P2/2R1QK2/8 w - - 4 6");
        //string testFenMat("8/8/1r1bnk2/1p1p1p2/8/1P1P1P2/1R1B1K2/8 w - - 0 5");
        //string testFenMat("8/8/1rnbqk2/1p1p1p2/3p4/1P1PPP2/1R1BQK2/8 b - - 0 3");
        string testFenAlamos("8/1rnqknr1/1pppppp1/8/8/1PPPPPP1/1RNQKNR1/8 w - - 0 1");
        //8/1rnqknr1/1pppppp1/8/8/1PPPPPP1/1RNQKNR1/8 w - - 0 1
        list<string> startingMoves;
        //Explore this bad move, demonstrate that black wins
        startingMoves.push_back("f2e4");
        /*
         *startingMoves.push_back("d3d4");
         *startingMoves.push_back("e5d4");
         *startingMoves.push_back("e3d4");
         *startingMoves.push_back("f5f4");
         */
        //startingMoves.push_back("c5d4");
        //Starting side should be read from fen
        int rt = theFinder->runFinder();
        //int rt = theFinder->runFinder(WHITE, startingMoves, testFenMat);
        Utils::handleError("runfinder()\n", rt);

        //Kill the child
        kill(pid, SIGTERM);
    } else {
        perror("Error: fork failed");
        exit(EXIT_FAILURE);
    }
    cout << "Exiting program\n";
    delete theFinder;
    return 0;
}
