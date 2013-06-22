#include <iostream>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include "MatFinder.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"


//TODO:
// - Write a chess board
// - Write error management
//

MatFinder::MatFinder(Engine chessEngine) : engine_(chessEngine)
{
    int pipe_status;

    // Create the pipes
    // We do this before the fork so both processes will know about
    // the same pipe and they can communicate.

    pipe_status = pipe(in_fds_);
    Utils::handleError("pipe() : Error creating the pipe", pipe_status);
    /*
     *if (pipe_status == -1)
     *{
     *    perror("Error creating the pipe");
     *    exit(EXIT_FAILURE);
     *}
     */

    pipe_status = pipe(out_fds_);
    Utils::handleError("pipe() : Error creating the pipe", pipe_status);
    /*
     *if (pipe_status == -1)
     *{
     *    perror("Error creating the pipe");
     *    exit(EXIT_FAILURE);
     *}
     */

    pipe_status = pipe(err_fds_);
    Utils::handleError("pipe() : Error creating the pipe", pipe_status);
    /*
     *if (pipe_status == -1)
     *{
     *    perror("Error creating the pipe");
     *    exit(EXIT_FAILURE);
     *}
     */
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

//FIXME: sideToMove should be read from fen, and move added
int MatFinder::runFinder(side_t enginePlayFor, list<string> &moves,
        string pos/* = "startpos"*/)
{
    //engine_input_.open(getEngineInWrite());
    side_t sideToMove = (pos == "startpos")?WHITE:Utils::getSideFromFen(pos);
    if (moves.size()%2)
        switchSide(&sideToMove);

    if (sideToMove == UNDEFINED)
        return 1;
    engine_side_ = sideToMove;
    engine_play_for_ = enginePlayFor;
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
    sendOptionToEngine("MultiPV", MAX_LINES_STR);
    sendToEngine("ucinewgame");

    sendToEngine("isready");
    //sendToEngine("position startpos moves e2e4 d7d5");
    //Expected move is e4d5
    //sendToEngine("go movetime 1000");

    waitReadyok();

    cout << "Starting side is " << SideNames[engine_side_] << endl;

    cout << "Doing some basic evaluation on submitted position..." << endl;
    sendCurrentPositionToEngine();
    sendToEngine("go movetime 5000");
    waitBestmove();
    cout << "Lines found are :" << endl;
    cout << getPrettyLines();

    //should be while true
    while (true) {
    //TODO: Write the engine pilot main loop...
    //End this !!!
        Line bestUnba;
        lines_.fill(Line::emptyLine);
        //final cond should take into account addedMoves_.empty()
        cout << "[" << SideNames[engine_side_] << "]" << endl;
        if (engine_side_ == engine_play_for_) {
            sendCurrentPositionToEngine();
            //We are now thinking
            sendToEngine("go movetime 2500");
            cout << "[" << SideNames[engine_side_] << "] Thinking... (2500)\n";
            waitBestmove();

            bestUnba = getFirstNotMatUnbalancedLine();
            if (bestUnba.empty()) {
                cout << "We just closed a line :" << endl;
                cout << getPrettyLines();
                //Handle the case where we should backtrack
                if (!addedMoves_.empty()) {
                    //Remove opposite previous move
                    addedMoves_.pop_back();
                    switchSide();
                    //Remove our previous move if we had one
                    //(not the case if starting side is not the side 
                    //the engine play for)
                    if (!addedMoves_.empty()) {
                        addedMoves_.pop_back();
                        switchSide();
                    }
                    continue;
                } else {
                    addedMoves_.clear();
                    //This is the end (hold your breath and count to ten)
                    break;
                }

                //FIXME: Handle error correctly
                Utils::handleError("Line empty from starting point of view",
                        1);
                break;
            }

            //TODO: refactor, common code !
            cout << "[" << SideNames[engine_side_] << "] Best line : \n";
            cout << "\t" << bestUnba.getPretty(engine_side_);
            cout << "\tNext move : ";
            string next = bestUnba.firstMove();
            cout << next << endl;
            addedMoves_.push_back(next);
            switchSide();

        } else {
            sendCurrentPositionToEngine();
            //We are now answering the "best" move, should not be so many
            //precise answers, so thinking should be short.
            sendToEngine("go movetime 1000");
            cout << "[" << SideNames[engine_side_] << "] Thinking... (1000)\n";
            waitBestmove();
            //Best line is the first
            bestUnba = lines_[0];
            if (bestUnba.empty()) {
                //FIXME: Handle error correctly
                Utils::handleError("Line empty from opposite point of view",
                        1);
                exit(EXIT_FAILURE);
            }
            if (bestUnba.isMat()) {
                if (addedMoves_.empty()) {
                    //The end, starting side was not playFor side
                    //FIXME: not needed, as white handle the case 
                    //where added.size() is 1
                    cout << "Looks like it's over" << endl;
                    break;
                }
                //Backtrack so that start is to move
                cout << "\tBacktrack move " << addedMoves_.back()
                    << " (addedMove#" << addedMoves_.size() << ")"
                    << endl;
                addedMoves_.pop_back();//remove *startingSide* move
                switchSide();
                continue;
            }
            //else just play the move for opponent

            cout << "[" << SideNames[engine_side_] << "] Best line : \n";
            cout << "\t" << bestUnba.getPretty(engine_side_);
            cout << "\tNext move : ";
            string next = bestUnba.firstMove();
            cout << next << endl;
            addedMoves_.push_back(next);
            switchSide();
        }
    }

    cout << "Finder is done." << endl;
    cout << "Starting side was " << SideNames[sideToMove] << endl;
    cout << "Engine played for " << SideNames[engine_play_for_] << endl;
    if (engine_play_for_ == sideToMove)
        cout << "All lines should now be draw or mat :" << endl;
    else
        cout << "Best line should be mat.\n" << endl;
    //Correct eval sign (last line are from black for this test)
    //switchSide();
    cout << getPrettyLines();



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
    //TODO: better this, debug mode
    string toSend(cmd);
    toSend += "\n";
    //cout << "Sending : " << toSend;
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
    if (!addedMoves_.empty()) {
        oss << "Lines after moves ";
        list<string> tmpList = startingMoves_;
        tmpList.insert(tmpList.end(), addedMoves_.begin(), addedMoves_.end());
        int i = 0;
        //TODO: #define max number of displayed move
        while (!tmpList.empty() && i < 8) {
            oss << tmpList.front();
            oss << " ";
            tmpList.pop_front();
            i++;
        }
        oss << "\n";
    }
    Line curLine;
    for (int i = 0; i < lines_.size(); ++i) {
        curLine = lines_[i];
        if (!curLine.empty()) {
            oss << "\t[";
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
    switchSide(&engine_side_);
}

void MatFinder::switchSide(side_t *side)
{
    (*side) = ((*side) == WHITE)?BLACK:WHITE;
}

void MatFinder::waitReadyok()
{
    pthread_mutex_lock(&readyok_mutex_);
    struct timespec ts;
    //TODO:#define it
    Utils::getTimeout(&ts, 5);
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
        //TODO: find a clearer way to define "balance"
        //eval is in centipawn, 100 ~ a pawn
        int limit = 50;
        if (fabs(lines_[i].getEval()) > limit && !lines_[i].isMat())
            return lines_[i];
    }
    //if all are draw, return the same line
    return Line::emptyLine;
}


