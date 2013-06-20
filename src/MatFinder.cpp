#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
#include "MatFinder.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"

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
    engine_input_ = new OutputStream(getEngineInWrite());
}

MatFinder::~MatFinder()
{
    // Close the unused ends of the pipes
    close(getEngineInRead());
    close(getEngineOutWrite());
    close(getEngineErrWrite());
    close(getEngineInWrite());
    close(getEngineOutRead());
    close(getEngineErrRead());
    delete engine_input_;
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
    //close(getEngineInWrite());
    //close(getEngineOutRead());
    //close(getEngineErrRead());

    //TODO: something with io ? Don't really need to...


    engine_.execEngine();
}

int MatFinder::runFinder(side_t engineSide, list<string> &moves,
        string pos/* = "startpos"*/)
{
    //engine_input_.open(getEngineInWrite());
    engine_side_ = engineSide;
    side_t startingSide = engineSide;
    startpos_ = pos;
    addedMoves_.clear();
    lines_.fill(Line::emptyLine);
    startingMoves_.clear();
    startingMoves_.insert(startingMoves_.end(), moves.begin(), moves.end());



    //Start the receiver
    Thread *thread = startReceiver();

    string message;

    cout << "Sending command" << endl;

    //Send some commands, init etc...
    sendToEngine("uci");
    //FIXME: #define these
    sendOptionToEngine("Hash", "4096");
    sendOptionToEngine("UCI_AnalyseMode", "true");
    sendOptionToEngine("MultiPV", "4");
    sendToEngine("ucinewgame");

    sendToEngine("isready");
    //sendToEngine("position startpos moves e2e4 d7d5");
    //Expected move is e4d5
    //sendToEngine("go movetime 1000");

    waitReadyok();



    int i = 0;

    //should be while true
    while (i < 4) {
    //TODO: Write the engine pilot main loop...
    //End this !!!
        Line bestUnba;
        //final cond should take into account addedMoves_.empty()
        if (engine_side_ == startingSide) {
            sendCurrentPositionToEngine();
            //We are now thinking
            sendToEngine("go movetime 5000");
            waitBestmove();

            bestUnba = getFirstNotMatUnbalancedLine();
            if (bestUnba.empty())
                break;

            cout << "[Start] Best line : \n";
            cout << bestUnba.getPretty(engine_side_);
            cout << "Next move : ";
            string next = bestUnba.firstMove();
            cout << next << endl;
            addedMoves_.push_back(next);
            switchSide();

        } else {
            sendCurrentPositionToEngine();
            //We are now answering the "best" move, should not be so many
            //precise answers, so thinking should be short.
            sendToEngine("go movetime 1000");
            waitBestmove();
            bestUnba = getFirstNotMatUnbalancedLine();
            if (bestUnba.empty())
                break;

            cout << "[Opposite] Best line : \n";
            cout << bestUnba.getPretty(engine_side_);
            cout << "Next move : ";
            string next = bestUnba.firstMove();
            cout << next << endl;
            addedMoves_.push_back(next);
            switchSide();
        }
        //Now compute exit, or not
        i++;
    }
    cout << "All lines should now be null or mat :\n";
    //Correct eval sign (last line are from black for this test)
    switchSide();
    cout << getPrettyLines();


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


    thread->kill();
    delete thread;
    return EXIT_SUCCESS;
}

void MatFinder::sendCurrentPositionToEngine()
{
    sendPositionToEngine(startpos_, startingMoves_, addedMoves_);
}

void MatFinder::sendPositionToEngine(string pos, list<string> &moves,
        list<string> &addedMoves)
{
    string position("position ");
    if (pos != "startpos")
        position += "fen ";
    position += pos;
    position += " ";
    list<string> tmp = moves;
    tmp.insert(tmp.end(), addedMoves.begin(), addedMoves.end());
    if (!tmp.empty())
        position += "moves ";
    while (!tmp.empty()) {
        position += tmp.front();
        position += " ";
        tmp.pop_front();
    }
    sendToEngine(position);
}

void MatFinder::sendOptionToEngine(string optionName, string optionValue)
{
    string option("setoption");
    option += " name ";
    option += optionName;
    option += " value ";
    option += optionValue;
    sendToEngine(option);
}

void MatFinder::sendToEngine(string cmd)
{
    //TODO: better this
    string toSend(cmd);
    toSend += "\n";
    cout << "Sending : " << toSend;
    (*engine_input_) << toSend;
}

void MatFinder::updateLine(int index, Line &line)
{
    if (index >= lines_.size()) {
        cerr << "Index out of bound !\n";
        return;
    }
    lines_[index].update(line);
}

void MatFinder::updateNps(int newNps)
{
    nps_ = newNps;
}

void MatFinder::updateThinktime(int newThinktime)
{
    thinktime_ = newThinktime;
}

void MatFinder::signalReadyok()
{
    pthread_mutex_lock(&readyok_mutex_);
    pthread_cond_signal(&readyok_cond_);
    pthread_mutex_unlock(&readyok_mutex_);
}

void MatFinder::signalBestmove(string &bestmove)
{
    //cout << "Bestmove is " << bestmove << endl;
    pthread_mutex_lock(&bestmove_mutex_);
    pthread_cond_signal(&bestmove_cond_);
    pthread_mutex_unlock(&bestmove_mutex_);
}

string MatFinder::getPrettyLines()
{
    ostringstream oss;
    oss << "Lines are :\n";
    Line curLine;
    for (int i = 0; i < lines_.size(); ++i) {
        curLine = lines_[i];
        if (!curLine.empty()) {
            oss << "[";
            oss << (i + 1);
            oss << "] ";
            oss << curLine.getPretty(engine_side_);
        }
    }
    return oss.str();
}

/*
 * These are private
 */

Thread *MatFinder::startReceiver()
{
    //Starts in separate thread, so that it's handled background
    UCIReceiver *task = new UCIReceiver(this);


    Thread *thread = new Thread(static_cast<Runnable *>(task));
    thread->start();
    thread->detach();
    return thread;
}

void MatFinder::switchSide()
{
    engine_side_ = (engine_side_ == WHITE)?BLACK:WHITE;
}

void MatFinder::waitReadyok()
{
    pthread_mutex_lock(&readyok_mutex_);
    struct timespec ts;
    //TODO:#define it
    getTimeout(&ts, 5);
    pthread_cond_timedwait(&readyok_cond_,
            &readyok_mutex_,
            &ts);
    pthread_mutex_unlock(&readyok_mutex_);
}

void MatFinder::waitBestmove()
{
    pthread_mutex_lock(&bestmove_mutex_);
    pthread_cond_wait(&bestmove_cond_, &bestmove_mutex_);
    pthread_mutex_unlock(&bestmove_mutex_);
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

Line &MatFinder::getFirstNotMatUnbalancedLine()
{
    for (int i = 0; i < lines_.size(); ++i) {
        if (lines_[i].getEval() != 0 && !lines_[i].isMat())
            return lines_[i];
    }
    //if all are null, return the same line
    return Line::emptyLine;
}

void MatFinder::getTimeout(struct timespec *ts, int seconds)
{
    struct timeval tp;
    int rc = gettimeofday(&tp, NULL);
    Utils::handleError("gettimeofday()\n", rc);

    /* Convert from timeval to timespec */
    ts->tv_sec  = tp.tv_sec;
    ts->tv_nsec = tp.tv_usec * 1000;
    ts->tv_sec += seconds;
}


