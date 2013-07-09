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
#ifndef __FINDER_H__
#define __FINDER_H__

#include <string>
#include <vector>
#include "Engine.h"
#include "Thread.h"
#include "Stream.h"
#include "Line.h"

using namespace std;

//Forward decl
class UCIReceiver;

class Finder {
public:
    Finder();
    ~Finder();
    int runFinder();
    int runEngine();

    //Used by the UCIReceiver
    void updateLine(int index, Line &line);
    void updateNps(int newNps);
    void updateThinktime(int newThinktime);
    int getEngineOutRead();
    void signalReadyok();
    void signalBestmove(string &bestmove);

    //General purpose methods
    //FIXME: make these private
    string getPrettyLines();
    string getPrettyLine(Line &line, int limit = -1);

protected:
    virtual int runFinderOnCurrentPosition() = 0;
    Thread *startReceiver();
    void sendCurrentPositionToEngine();
    void sendOptionToEngine(string optionName, string optionValue);
    void sendToEngine(string cmd);
    void waitReadyok();
    void waitBestmove();
    //TODO: think about getter's visibility...
    int getEngineInRead();
    int getEngineInWrite();
    int getEngineOutWrite();
    int getEngineErrRead();
    int getEngineErrWrite();


    int in_fds_[2], out_fds_[2], err_fds_[2];
    OutputStream *engine_input_;
    OutputStream *receiver_input_;
    UCIReceiver *uciReceiver_;
    pthread_cond_t readyok_cond_ = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t readyok_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t bestmove_cond_ = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t bestmove_mutex_ = PTHREAD_MUTEX_INITIALIZER;

    //Engine specific members
    Engine engine_;

    //The chessboard we play on
    Chessboard *cb_ = NULL;

    //The starting pos
    string startpos_ = "startpos";

    //NOTE: sorting is done by engine, according to the active side
    vector<Line> lines_;

    Board::Side engine_play_for_;

    //Number of moves "played" by the finder
    int addedMoves_;

    //TODO ?
    int nps_ = 0;
    int thinktime_ = 0;

};

#endif
