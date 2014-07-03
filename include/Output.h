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
#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <string>
#include <iostream>

namespace Err {
    void signalHandler(int rc);
    void handle(int rc);
    void handle(const std::string &msg);
    void handle(const std::string &caller, int rc);
    void handle(const std::string &msg, int rc,
                       const std::string &fileName, int lineNumber);
    void output(const std::string &msg);
}

namespace Out {
    void output(const std::string &msg, int level = 0);
    void output(std::string &dest, const std::string &msg, int level = 0);
    void output(std::ostream &out, const std::string &msg);
}
#endif
