#include "Engine.h"
#include <string>
#include <unistd.h>

using namespace std;

Engine::Engine(string engineName, string binPrefix) : 
    name_(engineName), prefix_(binPrefix)
{
}

void Engine::execEngine()
{
    string enginePath(prefix_);
    enginePath += "/";
    enginePath += name_;
    execl(enginePath.c_str(), name_.c_str(), (char*)NULL);
}

