#ifndef __ENGINE_H__
#define __ENGINE_H__
#include <string>
#include "Chessboard.h"
using namespace std;

class Engine {
public:
    Engine(const string engineName, const string binPrefix,
            const string startpos = Chessboard::CHESS_STARTPOS);
    void execEngine();
    const string getEngineStartpos();

private:
    const string name_;
    const string prefix_;
    string startpos_;
};

#endif
