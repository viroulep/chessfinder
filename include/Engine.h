#ifndef __ENGINE_H__
#define __ENGINE_H__
#include <string>
using namespace std;

class Engine {
public:
    Engine(const string engineName, const string binPrefix);
    void execEngine();

private:
    const string name_;
    const string prefix_;
};

#endif
