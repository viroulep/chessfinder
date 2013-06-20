#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <memory>

using namespace std;

class Runnable {
public:
    virtual void *run() = 0;
    virtual ~Runnable() = 0;
};


class Thread {
public:
    Thread(Runnable *run);
    virtual ~Thread();
    void start();
    void kill();
    void detach();
    void *join();
private:
    // thread ID
    pthread_t PthreadThreadID;
    pthread_attr_t threadAttribute;
    // runnable object will be deleted automatically
    Runnable *runnable_;
    Thread(const Thread&);
    const Thread& operator=(const Thread&);
    // called when run() completes
    void setCompleted();
    // stores return value from run()
    void *result;
    static void *startRunnable(void *pVoid);
    void printError(const string &msg, int status, const string &fileName, int lineNumber);
};

#endif
