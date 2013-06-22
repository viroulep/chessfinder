#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <memory>
#include <list>
#include "Thread.h"
#include "Engine.h"
#include "Utils.h"
#include "Line.h"
#include "MatFinder.h"

#define CHILD_STDIN_READ pipefds_input[0]
#define CHILD_STDIN_WRITE pipefds_input[1]
#define CHILD_STDOUT_READ pipefds_output[0]
#define CHILD_STDOUT_WRITE pipefds_output[1]
#define CHILD_STDERR_READ pipefds_error[0]
#define CHILD_STDERR_WRITE pipefds_error[1]

using namespace std;

// Child exec error signal
void exec_failed (int sig)
{
    cerr << "Exec failed. Child process couldn't be launched." << endl;
    exit (EXIT_FAILURE);
}

void parseArgs(int argc, char **argv)
{
    char *value = NULL;
    bool debugFlag = false;
    bool displayHelp = false;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "hdv:m:p:e:f:")) != -1) {
        switch (c) {
            //TODO
            case 'd':
                debugFlag = true;
                break;
            case 'h':
                displayHelp = true;
                break;
            case 'v':
                value = optarg;
                break;
            case '?':
                if (optopt == 'v')
                    Utils::handleErrorAndExit("Option -v requires an argument.");
                else if (isprint (optopt))
                    Utils::handleErrorAndExit("Unknown option : " + to_string(optopt) + ".");
                else
                    fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
            default:
                abort();
        }
    }
    if (displayHelp) {
        //DOIT
        exit(EXIT_SUCCESS);
    }

    //printf ("aflag = %d, bflag = %d, cvalue = %s\n",
            //aflag, bflag, cvalue);

    //for (index = optind; index < argc; index++)
        //printf ("Non-option argument %s\n", argv[index]);
}


int main(int argc, char **argv)
{
    parseArgs(argc, argv);
    //Option are : 
    //-f "fenstring"
    //-e "engine name"
    //-p "engine path"
    //-m "movestring"
    //-d "debug"
    //-v "varnumber"
    // Child error signal install
    struct sigaction action;
    action.sa_handler = exec_failed;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1, &action, NULL) < 0)
    {
        perror("SIGUSR1 install error");
        exit(EXIT_FAILURE);
    }

    Engine gardner("losalamosfish", "/usr/local/bin");
    MatFinder *theFinder = new MatFinder(gardner);

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
        cout << "The aim is to prove that all unbalanced lines lead to black mating white.";
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
        int rt = theFinder->runFinder(WHITE, startingMoves);
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
