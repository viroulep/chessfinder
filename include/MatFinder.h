#ifndef __MATFINDER_H__
#define __MATFINDER_H__

#include <string>
#include "Engine.h"
#include "Thread.h"
#include "Stream.h"


class MatFinder {
public:
    MatFinder(Engine chessEngine);
    ~MatFinder();
    int runFinder();
    int runEngine();

    void sendToEngine(string cmd);
    int getEngineOutRead();

private:
    shared_ptr<Thread> startReceiver();
    //NOTE: think about getter's visibility...
    int getEngineInRead();
    int getEngineInWrite();
    int getEngineOutWrite();
    int getEngineErrRead();
    int getEngineErrWrite();

    int in_fds_[2], out_fds_[2], err_fds_[2];
    Engine engine_;
    OutputStream engine_input_;
};

#endif
