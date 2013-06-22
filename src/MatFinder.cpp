#include <iostream>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include "MatFinder.h"
#include "MatFinderOptions.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"


//TODO:
// - Write a chess board
// - Write error management
//

MatFinder::MatFinder() :
    engine_(MatFinderOptions::getEngine(), MatFinderOptions::getPath())
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
    // Close the pipes
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


    engine_.execEngine();
}

int MatFinder::runFinder()
{
    string pos = MatFinderOptions::getStartingPos();
    startingMoves_.clear();
    startingMoves_.insert(startingMoves_.end(), 
            MatFinderOptions::getUserMoves().begin(),
            MatFinderOptions::getUserMoves().end());

    side_t sideToMove = (pos == "startpos")?WHITE:Utils::getSideFromFen(pos);
    if (startingMoves_.size()%2)
        switchSide(&sideToMove);

    if (sideToMove == UNDEFINED)
        return 1;
    engine_side_ = sideToMove;
    engine_play_for_ = MatFinderOptions::getPlayFor();
    startpos_ = pos;
    addedMoves_.clear();
    lines_.assign(MatFinderOptions::getMaxLines(), Line::emptyLine);



    //Start the receiver
    Thread *thread = startReceiver();

    string message;


    //Send some commands, init etc...
    sendToEngine("uci");
    sendOptionToEngine("Hash", to_string(MatFinderOptions::getHashmapSize()));
    sendOptionToEngine("UCI_AnalyseMode", "true");
    sendOptionToEngine("MultiPV", to_string(MatFinderOptions::getMaxLines()));
    sendToEngine("ucinewgame");

    sendToEngine("isready");

    waitReadyok();


    Utils::output("Starting side is " + string(SideNames[engine_side_]) + "\n");
    Utils::output("Doing some basic evaluation on submitted position...\n");

    sendCurrentPositionToEngine();
    sendToEngine("go movetime "
            + to_string(MatFinderOptions::getPlayforMovetime()));
    waitBestmove();
    Utils::output("Evaluation is :\n");
    Utils::output(getPrettyLines());

    //Main loop
    while (true) {
        Line bestLine;
        //Initialize vector with empty lines
        lines_.assign(MatFinderOptions::getMaxLines(), Line::emptyLine);
        Utils::output("[" + string(SideNames[engine_side_])
                + "] Depth " + to_string(addedMoves_.size()) + "\n");

        sendCurrentPositionToEngine();

        //Thinking according to the side the engine play for
        int moveTime = (engine_side_ == engine_play_for_) ?
            MatFinderOptions::getPlayforMovetime() :
            MatFinderOptions::getPlayagainstMovetime();
        sendToEngine("go movetime " + to_string(moveTime));

        Utils::output("[" + string(SideNames[engine_side_])
                + "] Thinking... (" + to_string(moveTime) + ")\n", 1);

        //Wait for engine to finish thinking
        waitBestmove();

        Utils::output(getPrettyLines(), 1);
        bestLine = getBestLine();
        if (bestLine.empty() || bestLine.isMat()) {
            //Handle the case where we should backtrack
            if (!addedMoves_.empty()) {
                Utils::output("\tBacktracking " + addedMoves_.back()
                    + " (addedMove#" + to_string(addedMoves_.size())
                    + ")\n");
                //Remove opposite side previous move
                addedMoves_.pop_back();
                switchSide();
                if (engine_side_ == engine_play_for_) {
                    //Remove our previous move if we had one, since the
                    //mat is "recorded" by engine
                    //(not the case if starting side is not the side 
                    //the engine play for)
                    if (!addedMoves_.empty()) {
                        addedMoves_.pop_back();
                        switchSide();
                    }
                }
                continue;
            } else {
                //This is the end (hold your breath and count to ten)
                break;
            }
        }

        Utils::output("[" + string(SideNames[engine_side_])
                + "] Chosen line : \n", 1);
        Utils::output("\t" + bestLine.getPretty(engine_side_), 1);
        string next = bestLine.firstMove();
        Utils::output("\tNext move is " + next, 2);
        addedMoves_.push_back(next);
        switchSide();
    }

    Utils::output("Finder is done.\n");
    Utils::output("Starting side was " + string(SideNames[sideToMove]) + "\n");
    Utils::output("Engine played for "
        + string(SideNames[engine_play_for_]) + "\n");
    if (engine_play_for_ == sideToMove)
        Utils::output("All lines should now be draw or mat :\n");
    else
        Utils::output("Best line should be mat or draw.\n");
    Utils::output(getPrettyLines());



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
    string toSend(cmd);
    toSend += "\n";
    Utils::output(cmd, 2);
    (*engine_input_) << toSend;
}

void MatFinder::updateLine(int index, Line &line)
{
    if (index >= lines_.size()) {
        Utils::handleError("Index out of bound !");
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
    Utils::output("Signaling readyok_cond", 5);
    pthread_cond_signal(&readyok_cond_);
    pthread_mutex_unlock(&readyok_mutex_);
}

void MatFinder::signalBestmove(string &bestmove)
{
    pthread_mutex_lock(&bestmove_mutex_);
    Utils::output("Signaling bestmove_cond", 5);
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
        while (!tmpList.empty() && i < MatFinderOptions::movesDisplayed) {
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
    Utils::getTimeout(&ts, MatFinderOptions::isreadyTimeout);
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

Line &MatFinder::getBestLine()
{
    for (int i = 0; i < lines_.size(); ++i) {
        int limit = 100;
        //FIXME: find a clearer way to define "balance"
        //eval is in centipawn, 100 ~ a pawn
        if (lines_[i].isMat()) {
            if (lines_[i].getEval() > 0)
                    /*NOTE: this cond can be used to explore all the lost
                     * line, if the side we play for is not the side starting
                     * to move !
                     *&& !(addedMoves_.size() == 0
                     *&& engine_side_ != engine_play_for_))
                     */
                //This line is a win !
                //eval is relative to engine side, ie positive eval is
                //good for us
                return lines_[i];
            //Else this line is lost and we don't wan't it
        } else {
            //If the line is a draw we don't want it
            if (fabs(lines_[i].getEval()) > limit)
                return lines_[i];
        }
    }
    //if all are draw, return the same line
    return Line::emptyLine;

}

