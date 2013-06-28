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
    pthread_t PthreadThreadID_;
    pthread_attr_t threadAttribute_;
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
