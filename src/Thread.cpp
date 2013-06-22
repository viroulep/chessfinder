#include <iostream>
#include <cassert>
#include <string.h>
#include "Thread.h"
#include "Utils.h"

using namespace std;

Runnable::~Runnable()
{}


Thread::Thread(Runnable *r) : 
        runnable_(r) {
    if(!runnable_){
        Utils::handleError("Thread::Thread(auto_ptr<Runnable> r,"\
                " bool isDetached) failed at " + string(__FILE__)
                + to_string(__LINE__) + " - runnable is NULL\n");
    }
}

Thread::~Thread()
{
    if (runnable_)
        delete runnable_;
}



void Thread::start() {
    // initialize attribute object
    int status = pthread_attr_init(&threadAttribute);
    Utils::handleError("pthread_attr_init failed at", status,
        __FILE__, __LINE__);

    // set the scheduling scope attribute
    status = pthread_attr_setscope(&threadAttribute,
                    PTHREAD_SCOPE_SYSTEM);
    Utils::handleError("pthread_attr_setscope failed at", status,
        __FILE__, __LINE__);

    status = pthread_create(&PthreadThreadID, &threadAttribute,
        Thread::startRunnable, (void*)this);    
    Utils::handleError("pthread_create failed at", status,
        __FILE__, __LINE__);

    status = pthread_attr_destroy(&threadAttribute);
    Utils::handleError("pthread_attr_destroy failed at", status,
        __FILE__, __LINE__);
}

void Thread::kill() 
{
    pthread_cancel(PthreadThreadID);
}


void Thread::detach()
{
    int status = pthread_detach(PthreadThreadID);
}

void *Thread::join()
{
    // A thread calling T.join() waits until thread T completes.
    int status = pthread_join(PthreadThreadID, NULL);
    // result was already saved by thread start function
    if(status) {
        Utils::handleError("pthread_join failed at", status,
            __FILE__, __LINE__);
    }
    return result;
}


void *Thread::startRunnable(void *pVoid) {
    // thread start the runnable function
    Thread* aThread = static_cast<Thread*>(pVoid);
    assert(aThread);
    aThread->result = aThread->runnable_->run();
    aThread->setCompleted();
    return aThread->result;
}


void Thread::setCompleted() {
// completion handled by pthread_join()
}

