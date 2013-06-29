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
#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <list>
#include <vector>
#include <sys/time.h>


using namespace std;

class Utils {
public:
    //implicitly inlined
    static void signalHandler(int rc);
    static void handleError(int rc);
    static void handleError(const string &msg);
    static void handleError(const string &caller, int rc);
    static void handleError(const string &msg, int rc,
            const string &fileName, int lineNumber);

    static void output(const string &msg, int level = 0);

    static string helpMessage();
    static string listToString(list<string> &theList);
    static void getTimeout(struct timespec *ts, int seconds);
    static const string RED;
    static const string RESET;

    //FIXME: should belong to board ?
    static int parseMovelist(list<string> &theList, string moves);
};
#endif
