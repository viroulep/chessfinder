#ifndef __MATFINDER_H__
#define __MATFINDER_H__

#include <string>
#include <vector>
#include "Engine.h"
#include "Thread.h"
#include "Stream.h"
#include "Line.h"

using namespace std;

class MatFinder {
public:
    MatFinder();
    ~MatFinder();
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

private:
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
    Line &getBestLine();


    int in_fds_[2], out_fds_[2], err_fds_[2];
    OutputStream *engine_input_;
    pthread_cond_t readyok_cond_ = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t readyok_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t bestmove_cond_ = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t bestmove_mutex_ = PTHREAD_MUTEX_INITIALIZER;

    //Engine specific members
    Engine engine_;
    Board::Side engine_play_for_;

    //The chessboard we play on
    Chessboard *cb_ = NULL;

    //Number of moves "played" by the finder
    int addedMoves_;

    //The starting pos
    string startpos_ = "startpos";

    //NOTE: sorting is done by engine, according to the active side
    vector<Line> lines_;

    //TODO ?
    int nps_ = 0;
    int thinktime_ = 0;
};

#endif
