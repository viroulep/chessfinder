#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include "UCICommunicator.h"

using namespace std;

namespace Comm {

    UCICommunicator::UCICommunicator(const EngineOptions &options) :
        optionsMap_(options)
    {
        cerr << "construct comm\n";
    }

    UCICommunicator::~UCICommunicator()
    {
        cerr << "destruct comm\n";
    }

    void UCICommunicator::sendOption(string name, string value)
    {
        send("setoption name " + name + " value " + value);
    }

    void UCICommunicator::quit()
    {
        send("quit");
    }

    void UCICommunicator::sendOptions()
    {
        for (auto option : optionsMap_)
            sendOption(option.first, option.second);
    }

    LocalUCICommunicator::LocalUCICommunicator(const string engineFullpath,
            const EngineOptions &options) :
        UCICommunicator(options), engineFullpath_(engineFullpath),
        engineName_(engineFullpath.substr(engineFullpath.find_last_of("/") + 1))
    {
        cerr << "construct localcomm\n";
    }

    LocalUCICommunicator::~LocalUCICommunicator()
    {
        cerr << "destruct localcomm\n";
        if (ok())
            kill(childPid_, SIGUSR1);
        else
            cerr << "engine was not running\n";
    }

    void *LocalUCICommunicator::run()
    {
        /*TODO setup communications*/
        pid_t pid = fork();
        if (pid == 0) {
            cerr << "Starting engine\n";
            execl(engineFullpath_.c_str(), engineName_.c_str(), (char *)'\0');

            // We should never reach this point
            // Tell the parent the exec failed
            kill(getppid(), SIGUSR1);
            cerr << "Engine execution failed\n";

        } else if (pid > 0) {
            childPid_ = pid;
            sleep(15);
            cerr << "coucou\n";
        } else {
            cerr << "couldn't fork\n";
        }
        return 0;
    }

    void LocalUCICommunicator::send(string cmd)
    {
        /*TODO*/

    }

    bool LocalUCICommunicator::ok()
    {
        int status;
        return (waitpid(childPid_, &status, WNOHANG) == 0);
    }

    template<class T>
        int UCICommunicatorPool::create(const string engineFullpath,
                const EngineOptions &options)
        {
            int id = __atomic_fetch_add(&currentId_, 1, __ATOMIC_SEQ_CST);
            pool_.insert(std::make_pair(id, new T(engineFullpath, options)));
            return id;
        }

    template int UCICommunicatorPool::create<LocalUCICommunicator>(
            const string engineFullpath,
            const EngineOptions &options);

    bool UCICommunicatorPool::destroy(int id)
    {
        if (pool_.count(id) > 0) {
            delete pool_[id];
            pool_.erase(id);
            return true;
        }
        return false;
    }

    UCICommunicator &UCICommunicatorPool::get(int id)
    {
        /*TODO throw something*/
        return *pool_[id];
    }

}
