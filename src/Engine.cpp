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
#include <string>
#include <unistd.h>
#include "Engine.h"

using namespace std;

Engine::Engine(const string engineName, const string binPrefix,
        const string startpos) :
    name_(engineName), prefix_(binPrefix), startpos_(startpos)
{
    //TODO: work on this ?
    if (engineName == "gardnerfish")
        startpos_ = Chessboard::GARDNER_STARTPOS;
    if (engineName == "losalamosfish" || engineName == "stockfishalamos")
        startpos_ = Chessboard::ALAMOS_STARTPOS;
}

const string Engine::getEngineStartpos()
{
    return startpos_;
}

void Engine::execEngine()
{
    string enginePath(prefix_);
    enginePath += "/";
    enginePath += name_;
    execl(enginePath.c_str(), name_.c_str(), (char*)NULL);
}

