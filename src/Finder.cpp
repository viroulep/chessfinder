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
#include "Finder.h"
#include "Options.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"



Finder::Finder() :
    engine_(Options::getEngine(), Options::getPath())
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
    receiver_input_ = new OutputStream(getEngineOutWrite());

    //Create our UCIReceiver
    uciReceiver_ = new UCIReceiver(this);


    //engine_side_ = cb_->getActiveSide();
    engine_play_for_ = Options::getPlayFor();
}

Finder::~Finder()
{
    // Close the pipes
    close(getEngineInRead());
    close(getEngineOutWrite());
    close(getEngineErrWrite());
    close(getEngineInWrite());
    close(getEngineOutRead());
    close(getEngineErrRead());
    delete engine_input_;
    delete receiver_input_;
    if (uciReceiver_)
        delete uciReceiver_;
}

int Finder::runEngine()
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

int Finder::runFinder()
{
    //Start the receiver
    Thread *thread = startReceiver();



    //Send some commands, init etc...
    sendToEngine("uci");
    sendOptionToEngine("Hash", to_string(Options::getHashmapSize()));
    sendOptionToEngine("UCI_AnalyseMode", "true");
    sendOptionToEngine("MultiPV", to_string(Options::getMaxLines()));
    sendOptionToEngine("Threads", to_string(Options::getEngineThreads()));
    sendToEngine("ucinewgame");

    sendToEngine("isready");

    waitReadyok();

    const PositionList &allPositions = Options::getPositionList();

    for (PositionList::const_iterator it = allPositions.begin(),
            itEnd = allPositions.end(); it != itEnd; ++it) {
        //Extract infos from pair
        string pos = (*it).first;
        string fenpos = (pos == "startpos")?engine_.getEngineStartpos():pos;
        list<string> userMoves = (*it).second;

        Utils::output("Running finder on \"" + pos + "\", with moves : "
                + Utils::listToString(userMoves) + "\n");


        //TODO: usereinit chessboard
        //Build chessboard
        cb_ = Chessboard::createFromFEN(fenpos);
        //Apply user moves
        cb_->uciApplyMoves(userMoves);

        //Fresh finder
        startpos_ = pos;
        addedMoves_ = 0;

        //Run
        runFinderOnCurrentPosition();

        delete cb_;
    }
    if (allPositions.empty())
        Utils::output("No position to run the finder on. Please adjust"\
                " --startingpos and/or --position_file\n");

    Utils::output("Exiting receiver and joining threads\n", 4);
    (*receiver_input_) << "quit\n";

    thread->join();
    delete thread;
    return EXIT_SUCCESS;
}

void Finder::updateLine(int index, Line &line)
{
    if (index >= lines_.size()) {
        Utils::handleError("Index '" + to_string(index) + "' out of bound !");
    }
    lines_[index].update(line);
}


void Finder::sendCurrentPositionToEngine()
{
    //TODO test this
    string fenPos = "position fen ";
    fenPos += cb_->exportToFEN();
    /*
     *string position("position ");
     *if (startpos_ != "startpos")
     *    position += "fen ";
     *position += startpos_;
     *position += " ";
     *const list<string> moves = cb_->getUciMoves();
     *if (!moves.empty())
     *    position += "moves ";
     *for (list<string>::const_iterator it = moves.begin(), itEnd = moves.end();
     *        it != itEnd; ++it)
     *    position += (*it) + " ";
     *sendToEngine(position);
     */
    sendToEngine(fenPos);
}

void Finder::sendOptionToEngine(string optionName, string optionValue)
{
    string option("setoption");
    option += " name ";
    option += optionName;
    option += " value ";
    option += optionValue;
    sendToEngine(option);
}

void Finder::sendToEngine(string cmd)
{
    string toSend(cmd);
    toSend += "\n";
    Utils::output(toSend, 3);
    (*engine_input_) << toSend;
}

void Finder::updateNps(int newNps)
{
    nps_ = newNps;
}

void Finder::updateThinktime(int newThinktime)
{
    thinktime_ = newThinktime;
}

void Finder::signalReadyok()
{
    pthread_mutex_lock(&readyok_mutex_);
    Utils::output("Signaling readyok_cond", 5);
    pthread_cond_signal(&readyok_cond_);
    pthread_mutex_unlock(&readyok_mutex_);
}

void Finder::signalBestmove(string &bestmove)
{
    pthread_mutex_lock(&bestmove_mutex_);
    Utils::output("Signaling bestmove_cond", 5);
    pthread_cond_signal(&bestmove_cond_);
    pthread_mutex_unlock(&bestmove_mutex_);
}

string Finder::getPrettyLines()
{
    ostringstream oss;
    Line curLine;
    for (int i = 0; i < lines_.size(); ++i) {
        curLine = lines_[i];
        if (!curLine.empty()) {
            oss << "\t[";
            oss << (i + 1);
            oss << "] ";
            oss << getPrettyLine(curLine, Options::movesDisplayed);
            oss << "\n";
        }
    }
    return oss.str();
}

string Finder::getPrettyLine(Line &line, int limit)
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

Thread *Finder::startReceiver()
{
    //Starts in separate thread, so that it's handled background
    Thread *thread = new Thread(static_cast<Runnable *>(uciReceiver_));
    thread->start();
    return thread;
}

void Finder::waitReadyok()
{
    pthread_mutex_lock(&readyok_mutex_);
    struct timespec ts;
    Utils::getTimeout(&ts, Options::isreadyTimeout);
    pthread_cond_timedwait(&readyok_cond_,
            &readyok_mutex_,
            &ts);
    pthread_mutex_unlock(&readyok_mutex_);
}

void Finder::waitBestmove()
{
    pthread_mutex_lock(&bestmove_mutex_);
    pthread_cond_wait(&bestmove_cond_, &bestmove_mutex_);
    pthread_mutex_unlock(&bestmove_mutex_);
}

int Finder::getEngineInRead()
{
    return in_fds_[0];
}

int Finder::getEngineInWrite()
{
    return in_fds_[1];
}

int Finder::getEngineOutRead()
{
    return out_fds_[0];
}

int Finder::getEngineOutWrite()
{
    return out_fds_[1];
}

int Finder::getEngineErrRead()
{
    return err_fds_[0];
}

int Finder::getEngineErrWrite()
{
    return err_fds_[1];
}

