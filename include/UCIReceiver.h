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
#ifndef __UCIRECEIVER_H__
#define __UCIRECEIVER_H__

#include <string>
#include "Stream.h"
#include "Finder.h"

/**
 * This class is responsible for parsing commands from engine
 * and updating the finder's state
 */
class UCIReceiver {
public:
    UCIReceiver(Finder *finder);
    ~UCIReceiver();
    void *run();
private:
    void bestmove(std::istringstream &is);
    void readyok(std::istringstream &is);
    void info(std::istringstream &is);
    void option(std::istringstream &is);
    InputStream *input_;
    Finder *finder_;
    int parseMessage(std::string msg);
    std::string strBuf_;
};

#endif
