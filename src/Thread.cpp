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
}



void Thread::start() {
    // initialize attribute object
    int status = pthread_attr_init(&threadAttribute_);
    Utils::handleError("pthread_attr_init failed at", status,
        __FILE__, __LINE__);

    // set the scheduling scope attribute
    status = pthread_attr_setscope(&threadAttribute_,
                    PTHREAD_SCOPE_SYSTEM);
    Utils::handleError("pthread_attr_setscope failed at", status,
        __FILE__, __LINE__);

    status = pthread_create(&PthreadThreadID_, &threadAttribute_,
        Thread::startRunnable, (void*)this);    
    Utils::handleError("pthread_create failed at", status,
        __FILE__, __LINE__);

    status = pthread_attr_destroy(&threadAttribute_);
    Utils::handleError("pthread_attr_destroy failed at", status,
        __FILE__, __LINE__);
}

void Thread::kill() 
{
    int status = pthread_cancel(PthreadThreadID_);
    Utils::handleError("pthread_cancel failed at", status,
        __FILE__, __LINE__);
}


void Thread::detach()
{
    int status = pthread_detach(PthreadThreadID_);
    Utils::handleError("pthread_detach failed at", status,
        __FILE__, __LINE__);
}

void *Thread::join()
{
    // A thread calling T.join() waits until thread T completes.
    int status = pthread_join(PthreadThreadID_, NULL);
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

