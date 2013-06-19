#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <memory>
#include "Thread.h"
#include "Engine.h"
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

    Engine stockfish("stockfish", "/usr/local/bin");
    MatFinder *theFinder = new MatFinder(stockfish);

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
    }
    else if (pid > pid_t(0))
    {
        cout << "Parent : running finder\n";
        theFinder->runFinder();
    }
    else
    {
        perror("Error: fork failed");
        exit(EXIT_FAILURE);
    }

    cout << "Exiting\n";
    delete theFinder;
    return 0;
}
