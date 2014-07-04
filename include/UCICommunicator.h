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
#ifndef __UCICOMMUNICATOR_H__
#define __UCICOMMUNICATOR_H__

#include <string>
#include <map>
#include <vector>
#include <pthread.h>

#include "Stream.h"
#include "Line.h"

namespace Comm {

    class EngineOptions : public std::map<std::string, std::string> {
        public:
            EngineOptions();
    };
    class UCICommunicatorPool;

    /**
     * This class is responsible for communicating with an engine.
     */
    class UCICommunicator {
        friend class UCICommunicatorPool;
        protected:
            UCICommunicator(const EngineOptions &options);
            virtual ~UCICommunicator();

            void sendOption(const std::string &name, const std::string &value) const;
            void sendOptions() const;
            const std::vector<Line> &getResultLines() const;

            /*Communicator specific*/
            virtual void *run() = 0;
            virtual bool ok() = 0;
            virtual bool send(const std::string &cmd) const = 0;
            virtual void quit() const;

            /*UCI response specific*/
            int parseUCIMsg(const std::string &msg);
            void bestmove(std::istringstream &is);
            void readyok(std::istringstream &is);
            void info(std::istringstream &is);
            bool waitBestmove();
            bool waitReadyok();
            void signalBestmove();
            void signalReadyok();
            void clearLines();

            pthread_cond_t readyok_cond_ = PTHREAD_COND_INITIALIZER;
            pthread_mutex_t readyok_mutex_ = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_t bestmove_cond_ = PTHREAD_COND_INITIALIZER;
            pthread_mutex_t bestmove_mutex_ = PTHREAD_MUTEX_INITIALIZER;

            /*pthread static routine*/
            static void *start_routine(void *arg);

            EngineOptions optionsMap_;
            std::vector<Line> linesVector_;
    };

    class LocalUCICommunicator : public UCICommunicator {
        friend class UCICommunicatorPool;
        private:
            LocalUCICommunicator(const std::string engineFullpath,
                    const EngineOptions &options);
            virtual ~LocalUCICommunicator();

            /*Communicator implementation*/
            virtual void *run();
            virtual bool send(const std::string &cmd) const;
            virtual bool ok();
            virtual void quit() const;

            /*LocalEngine specific*/
            int getEngineInRead();
            int getEngineInWrite();
            int getEngineOutRead();
            int getEngineOutWrite();
            int getEngineErrRead();
            int getEngineErrWrite();
            int in_fds_[2], out_fds_[2], err_fds_[2];
            pid_t childPid_ = 0;
            OutputStream *engine_input_;
            InputStream *engine_output_;
            OutputStream *receiver_input_;

            /*NOTE: maybe common to other implementations (ie: mpi)*/
            const std::string engineFullpath_;
            const std::string engineName_;
    };


    class UCICommunicatorPool {
        /*Singleton, threadsafe*/
        public:
            template<class T>
                int create(const std::string engineFullpath,
                        const EngineOptions &options);
            bool send(int id, const std::string &cmd);
            bool send(int id, const std::string &&c);
            bool send(int id, const char *c);
            bool isReady(int id);
            bool sendAndWaitBestmove(int id, const std::string &cmd);
            bool sendOption(int id, const std::string &name,
                            const std::string &value);
            const std::vector<Line> &getResultLines(int id);
            bool destroy(int id);
            bool destroyAll();
            static UCICommunicatorPool &getInstance();
        private:
            UCICommunicator *get(int id);
            bool waitForBestmove(int id);
            bool waitForReadyok(int id);
            UCICommunicatorPool &operator=(const UCICommunicatorPool &);
            UCICommunicatorPool(const UCICommunicatorPool &) {};
            UCICommunicatorPool() {};
            ~UCICommunicatorPool();

            /*
             * Map of all the UCICommunicator
             * It maps an id to an UCICommunicator and its managing thread
             */
            std::map<int, std::pair<UCICommunicator *, pthread_t>> pool_;

            /*
             * This is the current communicator id, should be manipulated
             * atomically
             */
            int currentId_ = 0;

            int destroyed_ = 0;

            /*The instance*/
            static UCICommunicatorPool instance_;
    };

}
#endif
