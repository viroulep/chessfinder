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

int main()
{
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

    Engine gardner("gardnerfish", "/usr/local/bin");
    MatFinder *theFinder = new MatFinder(gardner);

    pid_t pid;
    // Create child process; both processes continue from here
    pid = fork();

    if (pid == pid_t(0))
    {
        cout << "Child : running engine\n";
        theFinder->runEngine();

        // We should never reach this point
        // Tell the parent the exec failed
        kill(getppid(), SIGUSR1);
        exit(EXIT_FAILURE);
    } else if (pid > pid_t(0)) {
        //TODO: take position and moves as param !
        cout << "Parent : running finder\n";
        //string testFenMat("8/8/1rq2k2/1p1p1p2/1b1p1B2/1P1P1P2/2R1QK2/8 w - - 4 6");
        list<string> startingMoves;
        startingMoves.push_back("c2d4");
        startingMoves.push_back("c5d4");
        //Starting side should be read from fen
        int rt = theFinder->runFinder(WHITE, startingMoves);
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
