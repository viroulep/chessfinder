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

namespace Comm {

    typedef std::map<std::string, std::string> EngineOptions;

    /**
     * This class is responsible for communicating with an engine.
     */
    class UCICommunicator {
        public:
            UCICommunicator(const EngineOptions &options);
            virtual ~UCICommunicator();
            void sendOption(std::string name, std::string value);
            void waitBestmove();
            void waitReadyok();
            void quit();
            virtual void *run() = 0;
            virtual void send(std::string cmd) = 0;
            virtual bool ok() = 0;
        protected:
            void sendOptions();
            EngineOptions optionsMap_;
            /*
             *
             *    void bestmove(istringstream &is);
             *    void readyok(istringstream &is);
             *    void info(istringstream &is);
             *    void option(istringstream &is);
             *    InputStream *input_;
             *    Finder *finder_;
             *    int parseMessage(std::string msg);
             *    std::string strBuf_;
             */
    };

    class LocalUCICommunicator : public UCICommunicator {
        public:
            LocalUCICommunicator(const std::string engineFullpath,
                    const EngineOptions &options);
            virtual ~LocalUCICommunicator();
            virtual void *run();
            virtual void send(std::string cmd);
            virtual bool ok();
        private:
            const std::string engineFullpath_;
            const std::string engineName_;
            pid_t childPid_ = 0;
    };


    class UCICommunicatorPool {
        /*Must be threadsafe*/
        public:
            template<class T>
                int create(const std::string engineFullpath,
                        const EngineOptions &options);
            bool destroy(int id);
            UCICommunicator &get(int id);
        private:
            std::map<int, UCICommunicator *> pool_;
            int currentId_ = 0;
    };

}
#endif
