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
#include <sstream>
#include <fstream>
#include "UCICommunicator.h"
#include "Output.h"
#include "Options.h"

using namespace std;

namespace Err {

    void signalHandler(int rc)
    {
        handle("Engine execution failed : " + to_string(rc));
    }

    void handle(int rc)
    {
        handle("unspecified caller", rc);
    }

    void handle(const string &msg)
    {
        output(msg);
        /*
         *Ugly solution to bypass issues with data destruction when
         *    calling exit in a thread
         * TODO SIGUSR to exit all thread
         * + rework receiver and at exit
         */
        Comm::UCICommunicatorPool::getInstance().destroyAll();
        sleep(1);
        std::exit(EXIT_FAILURE);
    }

    void handle(const string &caller, int rc)
    {
        if (rc) {
            handle("Error on : " + caller + ", rc=" + to_string(rc));
        }
    }

    void handle(const string &msg, int rc,
            const string &fileName, int lineNumber)
    {
        if (rc)
            cerr << fileName << ":" << lineNumber << endl;
        handle(msg, rc);
    }


    void output(const string &msg)
    {
        Out::output(cerr, msg + "\n");
    }

}

namespace Out {

    void output(const string &msg, int level/* = 0*/)
    {
        if (level <= Options::getInstance().getVerboseLevel())
            output(cout, msg);
    }

    void output(string &dest, const string &msg, int level/* = 0*/)
    {
        if (level <= Options::getInstance().getVerboseLevel())
            dest += msg;
    }

    void output(std::ostream &out, const std::string &msg)
    {
        out << msg << flush;
    }

}
