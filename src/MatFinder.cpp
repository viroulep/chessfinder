#include <iostream>
#include <unistd.h>
#include "MatFinder.h"
#include "Stream.h"
#include "UCIReceiver.h"

MatFinder::MatFinder(Engine chessEngine) : engine_(chessEngine)
{
    int pipe_status;

    // Create the pipes
    // We do this before the fork so both processes will know about
    // the same pipe and they can communicate.

    pipe_status = pipe(in_fds_);
    if (pipe_status == -1)
    {
        perror("Error creating the pipe");
        exit(EXIT_FAILURE);
    }

    pipe_status = pipe(out_fds_);
    if (pipe_status == -1)
    {
        perror("Error creating the pipe");
        exit(EXIT_FAILURE);
    }

    pipe_status = pipe(err_fds_);
    if (pipe_status == -1)
    {
        perror("Error creating the pipe");
        exit(EXIT_FAILURE);
    }
}

MatFinder::~MatFinder()
{
}

int MatFinder::runEngine()
{
    // Engine part of the process
    // Its only duty is to run the chessengine

    // Tie the standard input, output and error streams to the
    // appropiate pipe ends
    // The file descriptor 0 is the standard input
    // We tie it to the read end of the pipe as we will use
    // this end of the pipe to read from it
    dup2 (getEngineInRead(),0);
    dup2 (getEngineOutWrite(),1);
    dup2 (getEngineErrWrite(),2);

    // Close the unused ends of the pipes
    close(getEngineInWrite());
    close(getEngineOutRead());
    close(getEngineErrRead());

    //TODO: something with io ? Don't really need to...


    engine_.execEngine();
}

int MatFinder::runFinder()
{
    engine_input_.open(getEngineInWrite());

    // Close the unused ends of the pipes
    close(getEngineInRead());
    close(getEngineOutWrite());
    close(getEngineErrWrite());


    //Start the receiver
    shared_ptr<Thread> thread = startReceiver();

    string message;

    cout << "Sending command" << endl;

    //Send some commands, test
    //TODO: Write the engine pilot...
    sendToEngine("uci");
    sendToEngine("position startpos moves e2e4 d7d5");
    //Expected move is e4d5
    sendToEngine("go movetime 1000");


    /*
       idée du programme :
       pos de départ, 
       un move blanc
       si eval != 0
       explorer les mats
       jouer 'top' coup noir en < 1000ms
       explorer les coups blancs
       while (eval variation 1 in -300 300)
       lister les variations : si eval < 300 ou > 300 : mat
       si rien en ??s, "aider"
       si eval = 0
       ??
     * */


    thread->join();
    close(getEngineInWrite());
    close(getEngineOutRead());
    close(getEngineErrRead());
    cout << "Exiting\n";
    exit(EXIT_SUCCESS);
}

void MatFinder::sendToEngine(string cmd)
{
    //TODO: better this
    string toSend(cmd);
    toSend += "\n";
    engine_input_ << toSend;
}

shared_ptr<Thread> MatFinder::startReceiver()
{
    //Starts in separate thread, so that it's handled background
    shared_ptr<UCIReceiver> task(new UCIReceiver(this));


    shared_ptr<Thread> thread(new Thread(static_pointer_cast<Runnable>(task)));
    thread->start();
    return thread;
}


int MatFinder::getEngineInRead()
{
    return in_fds_[0];
}

int MatFinder::getEngineInWrite()
{
    return in_fds_[1];
}

int MatFinder::getEngineOutRead()
{
    return out_fds_[0];
}

int MatFinder::getEngineOutWrite()
{
    return out_fds_[1];
}

int MatFinder::getEngineErrRead()
{
    return err_fds_[0];
}

int MatFinder::getEngineErrWrite()
{
    return err_fds_[1];
}



