#include <string>
#include <unistd.h>
#include "Engine.h"

using namespace std;

Engine::Engine(const string engineName, const string binPrefix,
        const string startpos) : 
    name_(engineName), prefix_(binPrefix), startpos_(startpos)
{
    if (engineName == "gardnerfish")
        startpos_ = Chessboard::GARDNER_STARTPOS;
    if (engineName == "alamosfish")
        startpos_ = Chessboard::ALAMOS_STARTPOS;
}

const string Engine::getEngineStartpos()
{
    return startpos_;
}

void Engine::execEngine()
{
    string enginePath(prefix_);
    enginePath += "/";
    enginePath += name_;
    execl(enginePath.c_str(), name_.c_str(), (char*)NULL);
}

