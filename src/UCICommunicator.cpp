#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <set>
#include "UCICommunicator.h"
#include "Output.h"

using namespace std;

namespace Comm {

    namespace Utils {
        void getTimeout(struct timespec *ts, int seconds)
        {
            struct timeval tp;
            int rc = gettimeofday(&tp, NULL);
            Err::handle("gettimeofday()", rc);

            /* Convert from timeval to timespec */
            ts->tv_sec  = tp.tv_sec;
            ts->tv_nsec = tp.tv_usec * 1000;
            ts->tv_sec += seconds;
        }

    }

    UCICommunicator::UCICommunicator(const EngineOptions &options) :
        optionsMap_(options)
    {
        clearLines();
    }

    UCICommunicator::~UCICommunicator()
    {
    }

    void UCICommunicator::sendOption(const string &name, const string &value) const
    {
        send("setoption name " + name + " value " + value);
    }

    void UCICommunicator::sendOptions() const
    {
        for (auto option : optionsMap_)
            sendOption(option.first, option.second);
    }


    const std::vector<Line> &UCICommunicator::getResultLines() const
    {
        return linesVector_;
    }

    void UCICommunicator::quit() const
    {
        send("quit");
    }

    int UCICommunicator::parseUCIMsg(const string &msg)
    {
        /*cerr << "Received UCI msg : \"" << msg << "\"\n";*/
        string token;
        //cerr << "\tcmd: " << msg << "\n";
        istringstream is(msg);
        is >> skipws >> token;
        /*Some of the tokens are just drop, because we don't care about them*/
        if (token == "id") ;
        else if (token == "uciok") ;
        else if (token == "quit") return 1;
        else if (token == "bestmove") bestmove(is);
        else if (token == "readyok") readyok(is);
        else if (token == "info") info(is);
        else if (token == "option") ;
        else {
            /*Err::output("Unrecognise command from engine :");*/
            Err::output("\"" + msg + "\"");
        }
        return 0;
    }

    void UCICommunicator::bestmove(istringstream &is)
    {
        string bm;
        //consume bestmove
        is >> bm;
        signalBestmove();
    }

    void UCICommunicator::readyok(istringstream &)
    {
        signalReadyok();
    }

    void UCICommunicator::info(istringstream &is)
    {
        string token;
        bool readLine = false;
        bool eval = false;
        int curDepth = 0;
        int curNps = 0;
        int curThinktime = 0;
        float curEval = 0;
        bool curIsMat = false;
        string curMv;
        list<string> curMoves;
        unsigned int curLineId = 0;
        while (is >> token) {
            if (token == "depth") {
                is >> curDepth;
            } else if (token == "seldepth") {
                //Drop
            } else if (token == "time") {
                is >> curThinktime;
                /*FIXME restore*/
                /*Out::output("Updated current thinktime to "*/
                            /*+ to_string(curThinktime) + "\n", 5);*/
            } else if (token == "nodes") {
                //Drop
            } else if (token == "pv") {
                readLine = true;
                //NOTE: should be the last token of infoline
                while (is >> token)
                    curMoves.push_back(token);
            } else if (token == "multipv") {
                is >> curLineId;
            } else if (token == "score") {
                eval = true;
                is >> token;
                if (token == "mate") {
                    curIsMat = true;
                    is >> curEval;
                } else if (token == "cp") {
                    is >> curEval;
                } else {
                    Err::output("********* No score ***********");
                }
            } else if (token == "currmove") {
                //Drop
            } else if (token == "currmovenumber") {
                //Drop
            } else if (token == "hashfull") {
                //NOTE: not supported by stockfish
                is >> token;
                Err::output("********** Hashfull : " + token + " *******");
            } else if (token == "nps") {
                is >> curNps;
                /*FIXME restore*/
                /*Out::output("Updated NPS to " + to_string(curNps) + "\n", 5);*/
            } else if (token == "tbhits") {
                //Drop
            } else if (token == "cpuload") {
                //Drop
            } else if (token == "string") {
                //Drop
            } else if (token == "refutation") {
                //Drop
            } else if (token == "currline") {
                //Drop
            }
        }
        if (readLine) {
            if (!eval || curMoves.empty() || !curLineId) {
                Err::output("**** Line without eval or move... ****");
                return;
            }
            Line curLine(curEval, curDepth, curMoves, curIsMat);
            //Update the line
            if (curLineId > linesVector_.size())
                Err::handle("Line index out of bound, please adjust MaxMoves"
                            "above " + to_string(curLineId));
            linesVector_[curLineId - 1] = curLine;
        }
    }

    bool UCICommunicator::waitBestmove()
    {
        pthread_mutex_lock(&bestmove_mutex_);
        pthread_cond_wait(&bestmove_cond_, &bestmove_mutex_);
        pthread_mutex_unlock(&bestmove_mutex_);
        return true;
    }

    bool UCICommunicator::waitReadyok()
    {
        pthread_mutex_lock(&readyok_mutex_);
        struct timespec ts;
        /*FIXME get timeout from options*/
        Utils::getTimeout(&ts, 5);
        pthread_cond_timedwait(&readyok_cond_,
                &readyok_mutex_,
                &ts);
        pthread_mutex_unlock(&readyok_mutex_);
        return true;
    }

    void UCICommunicator::signalBestmove()
    {
        pthread_mutex_lock(&bestmove_mutex_);
        Out::output("Signaling bestmove_cond\n", 5);
        pthread_cond_signal(&bestmove_cond_);
        pthread_mutex_unlock(&bestmove_mutex_);
    }

    void UCICommunicator::signalReadyok()
    {
        pthread_mutex_lock(&readyok_mutex_);
        Out::output("Signaling readyok_cond\n", 5);
        pthread_cond_signal(&readyok_cond_);
        pthread_mutex_unlock(&readyok_mutex_);
    }

    void UCICommunicator::clearLines()
    {
        /*FIXME get max moves from option*/
        linesVector_.assign(254, Line());
    }

    void *UCICommunicator::start_routine(void *arg)
    {
        UCICommunicator *comm = static_cast<UCICommunicator *>(arg);
        if (comm) {
            comm->run();
            return 0;
        } else {
            return (void *)1;
        }
    }

    LocalUCICommunicator::LocalUCICommunicator(const string engineFullpath,
            const EngineOptions &options) :
        UCICommunicator(options), engineFullpath_(engineFullpath),
        engineName_(engineFullpath.substr(engineFullpath.find_last_of("/") + 1))
    {
        int pipe_status;

        // Create the pipes
        // We do this before the fork so both processes will know about
        // the same pipe and they can communicate.

        pipe_status = pipe(in_fds_);
        Err::handle("pipe() : Error creating the pipe", pipe_status);


        pipe_status = pipe(out_fds_);
        Err::handle("pipe() : Error creating the pipe", pipe_status);

        pipe_status = pipe(err_fds_);
        Err::handle("pipe() : Error creating the pipe", pipe_status);

        engine_input_ = new OutputStream(getEngineInWrite());
        engine_output_ = new InputStream(getEngineOutRead());
        receiver_input_ = new OutputStream(getEngineOutWrite());
    }

    LocalUCICommunicator::~LocalUCICommunicator()
    {
        if (ok())
            kill(childPid_, SIGUSR1);
        else
            Err::output("engine was not running");
        close(getEngineInRead());
        close(getEngineOutWrite());
        close(getEngineErrWrite());
        close(getEngineInWrite());
        close(getEngineOutRead());
        close(getEngineErrRead());
        delete engine_input_;
        delete engine_output_;
        delete receiver_input_;
    }

    void *LocalUCICommunicator::run()
    {
        /*TODO setup communications*/
        pid_t pid = fork();
        if (pid == 0) {
            dup2(getEngineInRead(), 0);
            dup2(getEngineOutWrite(), 1);
            dup2(getEngineErrWrite(), 2);
            execl(engineFullpath_.c_str(), engineName_.c_str(), (char *)'\0');

            // We should never reach this point
            // Tell the parent the exec failed
            kill(getppid(), SIGUSR1);
            Err::handle("Engine execution failed");

        } else if (pid > 0) {
            childPid_ = pid;
            //Should get each engine message, then parse it
            //then eventually update and notify matFinder
            string strBuf;
            while (true) {
                getline((*engine_output_), strBuf);
                if (parseUCIMsg(strBuf))
                    break;
                /*if (strBuf == "quit\n")*/
                    /*break;*/
            }
        } else {
            Err::handle("couldn't fork");
        }
        return 0;
    }

    bool LocalUCICommunicator::send(const string &cmd) const
    {
        Out::output("Sending \"" + cmd + "\" to engine !\n", 5);
        (*engine_input_) << cmd << "\n";
        return true;
    }

    bool LocalUCICommunicator::ok()
    {
        int status;
        return (waitpid(childPid_, &status, WNOHANG) == 0);
    }

    void LocalUCICommunicator::quit() const
    {
        UCICommunicator::quit();
        (*receiver_input_) << "quit\n";
    }

    int LocalUCICommunicator::getEngineInRead()
    {
        return in_fds_[0];
    }

    int LocalUCICommunicator::getEngineInWrite()
    {
        return in_fds_[1];
    }

    int LocalUCICommunicator::getEngineOutRead()
    {
        return out_fds_[0];
    }

    int LocalUCICommunicator::getEngineOutWrite()
    {
        return out_fds_[1];
    }

    int LocalUCICommunicator::getEngineErrRead()
    {
        return err_fds_[0];
    }

    int LocalUCICommunicator::getEngineErrWrite()
    {
        return err_fds_[1];
    }

    template<class T>
        int UCICommunicatorPool::create(const string engineFullpath,
                const EngineOptions &options)
        {
            int id = __atomic_fetch_add(&currentId_, 1, __ATOMIC_SEQ_CST);
            T *engineComm = new T(engineFullpath, options);

            pool_.insert(make_pair(id, make_pair(engineComm, pthread_t())));

            pthread_t *tid = &pool_[id].second;
            int status = pthread_create(tid, 0, UCICommunicator::start_routine,
                           (void *)pool_[id].first);
            Err::handle("pthread_create", status);

            /*FIXME globalize*/
            unsigned int pollTime = 20000;
            while (!isReady(id))
                usleep(pollTime);

            engineComm->send("uci");
            engineComm->sendOptions();

            return id;
        }

    template int UCICommunicatorPool::create<LocalUCICommunicator>(
            const string engineFullpath,
            const EngineOptions &options);

    bool UCICommunicatorPool::send(int id, const string &cmd)
    {
        UCICommunicator *engine;
        return ((engine = get(id)) && engine->send(cmd));
    }

    bool UCICommunicatorPool::send(int id, const std::string &&c)
    {
        string cmd(c);
        return send(id, cmd);
    }

    bool UCICommunicatorPool::send(int id, const char *c)
    {
        string cmd(c);
        return send(id, cmd);
    }

    bool UCICommunicatorPool::isReady(int id)
    {
        return (send(id, "isready")) ? waitForReadyok(id) : false;
    }

    bool UCICommunicatorPool::sendAndWaitBestmove(int id, const std::string &cmd)
    {
        UCICommunicator *engine;
        if ((engine = get(id)))
            engine->clearLines();
        return (send(id, cmd)) ? waitForBestmove(id) : false;
    }

    const vector<Line> &UCICommunicatorPool::getResultLines(int id)
    {
        UCICommunicator *engine;
        if (!(engine = get(id)))
            Err::handle("Error : no engine \"" + to_string(id)
                        + "\" to get the results from !");
        return engine->getResultLines();
    }

    bool UCICommunicatorPool::destroy(int id)
    {
        if (pool_.count(id) > 0) {
            /*Properly exit engine*/
            pool_[id].first->quit();
            void **retVal = nullptr;
            int status = pthread_join(pool_[id].second, retVal);
            Err::handle("pthread_join", status);
            /*Remove Communicator from the pool*/
            delete pool_[id].first;
            pool_.erase(id);
            return true;
        }
        return false;
    }

    bool UCICommunicatorPool::destroyAll()
    {
        bool done = true;
        /*Necessary to prevent iterator from being invalidated*/
        set<int> toDestroy;
        for (auto entry : pool_)
            toDestroy.insert(entry.first);
        for (int id : toDestroy) {
            Out::output("destroying " + to_string(id) + "\n", 5);
            done &= destroy(id);
        }
        return done;
    }

    UCICommunicatorPool &UCICommunicatorPool::getInstance()
    {
        return instance_;
    }

    /*Private methods*/

    UCICommunicator *UCICommunicatorPool::get(int id)
    {
        if (pool_.count(id) > 0 && pool_[id].first->ok())
            return pool_[id].first;
        else {
            /*FIXME display error ?*/
            /*Err::output("Unknown UCICommunicator id : " + to_string(id));*/
            return nullptr;
        }
    }

    bool UCICommunicatorPool::waitForBestmove(int id)
    {
        UCICommunicator *engine;
        return ((engine = get(id)) && engine->waitBestmove());
    }

    bool UCICommunicatorPool::waitForReadyok(int id)
    {
        UCICommunicator *engine;
        return ((engine = get(id)) && engine->waitReadyok());
    }

    UCICommunicatorPool &UCICommunicatorPool::operator=(const UCICommunicatorPool &)
    {
        return getInstance();
    }

    UCICommunicatorPool::~UCICommunicatorPool()
    {
        destroyAll();
    }

    UCICommunicatorPool UCICommunicatorPool::instance_ = UCICommunicatorPool();

}
