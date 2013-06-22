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

    //should update or create the line and update the gui
    void updateLine(int index, Line &line);
    void updateNps(int newNps);
    void updateThinktime(int newThinktime);
    int getEngineOutRead();
    void signalReadyok();
    void signalBestmove(string &bestmove);
    string getPrettyLines();

private:
    Thread *startReceiver();
    void switchSide();
    void switchSide(side_t *side);
    void sendCurrentPositionToEngine();
    void sendPositionToEngine(string pos, list<string> &moves,
            list<string> &addedMoves);
    void sendOptionToEngine(string optionName, string optionValue);
    void sendToEngine(string cmd);
    void waitReadyok();
    void waitBestmove();
    //NOTE: think about getter's visibility...
    int getEngineInRead();
    int getEngineInWrite();
    int getEngineOutWrite();
    int getEngineErrRead();
    int getEngineErrWrite();
    Line &getBestLine();
    Line &getFirstNotMatUnbalancedLine();
    Line &getFirstUnbalancedLine();


    int in_fds_[2], out_fds_[2], err_fds_[2];
    OutputStream *engine_input_;
    pthread_cond_t readyok_cond_ = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t readyok_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t bestmove_cond_ = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t bestmove_mutex_ = PTHREAD_MUTEX_INITIALIZER;

    //Engine specific members
    Engine engine_;
    side_t engine_play_for_ = UNDEFINED;
    side_t engine_side_ = UNDEFINED;
    //The starting moves (might be defined by user)
    list<string> startingMoves_;
    //Move "played" by the finder
    list<string> addedMoves_;
    //The starting pos
    string startpos_ = "startpos";
    //NOTE: sort the array according to eval
    //smallest (absolute value) to greatest
    //NOTEbis : might already be done by engine
    //array<Line, MAX_LINES> lines_;
    vector<Line> lines_;
    int nps_ = 0;
    int thinktime_ = 0;
};

#endif
