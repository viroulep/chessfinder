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
#include <sstream>
#include <cmath>
#include <unistd.h>
#include "MatFinder.h"
#include "MatFinderOptions.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"



MatFinder::MatFinder() :
    engine_(MatFinderOptions::getEngine(), MatFinderOptions::getPath())
{
    int pipe_status;

    // Create the pipes
    // We do this before the fork so both processes will know about
    // the same pipe and they can communicate.

    pipe_status = pipe(in_fds_);
    Utils::handleError("pipe() : Error creating the pipe", pipe_status);

    pipe_status = pipe(out_fds_);
    Utils::handleError("pipe() : Error creating the pipe", pipe_status);

    pipe_status = pipe(err_fds_);
    Utils::handleError("pipe() : Error creating the pipe", pipe_status);

    engine_input_ = new OutputStream(getEngineInWrite());

    //Create our UCIReceiver
    uciReceiver_ = new UCIReceiver(this);

    //Build chessboard
    string pos = MatFinderOptions::getStartingPos();
    startpos_ = pos;
    addedMoves_ = 0;

    string fenpos = (pos == "startpos")?engine_.getEngineStartpos():pos;
    cb_ = Chessboard::createFromFEN(fenpos);

    //Apply user moves
    cb_->uciApplyMoves(MatFinderOptions::getUserMoves());


    //engine_side_ = cb_->getActiveSide();
    engine_play_for_ = MatFinderOptions::getPlayFor();

    lines_.assign(MatFinderOptions::getMaxLines(), Line::emptyLine);
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
    if (uciReceiver_)
        delete uciReceiver_;
    if (cb_)
        delete cb_;
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
    return 0;
}

int MatFinder::runFinder()
{

    Board::Side sideToMove = cb_->getActiveSide();

    //Start the receiver
    Thread *thread = startReceiver();

    //Send some commands, init etc...
    sendToEngine("uci");
    sendOptionToEngine("Hash", to_string(MatFinderOptions::getHashmapSize()));
    sendOptionToEngine("UCI_AnalyseMode", "true");
    sendOptionToEngine("MultiPV", to_string(MatFinderOptions::getMaxLines()));
    sendToEngine("ucinewgame");

    sendToEngine("isready");

    waitReadyok();


    Utils::output("Starting board is :\n" + cb_->to_string() + "\n");
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
        Side active = cb_->getActiveSide();
        //Initialize vector with empty lines
        lines_.assign(MatFinderOptions::getMaxLines(), Line::emptyLine);
        Utils::output("[" + Board::to_string(active)
                + "] Depth " + to_string(addedMoves_) + "\n");

        sendCurrentPositionToEngine();

        Utils::output(cb_->to_string(), 2);

        //Thinking according to the side the engine play for
        int moveTime = (active == engine_play_for_ || !addedMoves_) ?
            MatFinderOptions::getPlayforMovetime() :
            MatFinderOptions::getPlayagainstMovetime();
        sendToEngine("go movetime " + to_string(moveTime));

        Utils::output("[" + Board::to_string(active)
                + "] Thinking... (" + to_string(moveTime) + ")\n", 1);

        //Wait for engine to finish thinking
        waitBestmove();

        Utils::output(getPrettyLines(), 2);
        bestLine = getBestLine();
        if (bestLine.empty() || bestLine.isMat()) {
            //Handle the case where we should backtrack
            if (addedMoves_ > 0) {
                Utils::output("\tBacktracking " + cb_->getUciMoves().back()
                    + " (addedMove#" + to_string(addedMoves_)
                    + ")\n");
                //Remove opposite side previous move
                addedMoves_--;
                cb_->undoMove();

                if (active == engine_play_for_) {
                    //Remove our previous move if we had one, since the
                    //mat is "recorded" by engine
                    //(not the case if starting side is not the side 
                    //the engine play for)
                    if (addedMoves_ > 0) {
                        addedMoves_--;
                        cb_->undoMove();
                    }
                }
                continue;
            } else {
                //This is the end (hold your breath and count to ten)
                break;
            }
        }

        //If we are here, we just need to handle the next move
        Utils::output("[" + Board::to_string(active)
                + "] Chosen line : \n", 1);
        Utils::output("\t" + getPrettyLine(bestLine,
                    MatFinderOptions::movesDisplayed) + "\n", 1);

        string next = bestLine.firstMove();
        Utils::output("\tNext move is " + next + "\n", 3);
        addedMoves_++;
        cb_->uciApplyMove(next);
    }

    //Display info at the end of computation
    Utils::output("Finder is done. Starting board was : \n");
    Utils::output(cb_->to_string() + "\n");

    if (engine_play_for_ == sideToMove)
        Utils::output("All lines should now be draw or mat :\n");
    else
        Utils::output("Best line should be mat or draw.\n");
    Utils::output(getPrettyLines());
    Utils::output("Full best line is : \n");
    Utils::output(getPrettyLine(lines_[0]) + "\n");



    thread->kill();
    delete thread;
    return EXIT_SUCCESS;
}

void MatFinder::sendCurrentPositionToEngine()
{
    string position("position ");
    if (startpos_ != "startpos")
        position += "fen ";
    position += startpos_;
    position += " ";
    const list<string> moves = cb_->getUciMoves();
    if (!moves.empty())
        position += "moves ";
    for (list<string>::const_iterator it = moves.begin(), itEnd = moves.end();
            it != itEnd; ++it)
        position += (*it) + " ";
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
    Utils::output(cmd, 3);
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
    Line curLine;
    for (int i = 0; i < lines_.size(); ++i) {
        curLine = lines_[i];
        if (!curLine.empty()) {
            oss << "\t[";
            oss << (i + 1);
            oss << "] ";
            oss << getPrettyLine(curLine, MatFinderOptions::movesDisplayed);
            oss << "\n";
        }
    }
    return oss.str();
}

string MatFinder::getPrettyLine(Line &line, int limit)
{
    ostringstream oss;
    oss << line.getPrettyEval(cb_->getActiveSide() == Side::BLACK);
    oss << " : ";
    oss << cb_->tryUciMoves(line.getMoves(), limit);
    return oss.str();
}

/*
 * These are private
 */

Thread *MatFinder::startReceiver()
{
    //Starts in separate thread, so that it's handled background
    Thread *thread = new Thread(static_cast<Runnable *>(uciReceiver_));
    thread->start();
    thread->detach();
    return thread;
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

/**
 * This function determine the "best" line to follow
 */
Line &MatFinder::getBestLine()
{
    for (int i = 0; i < lines_.size(); ++i) {
        int limit = MatFinderOptions::getCpTreshold();
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

