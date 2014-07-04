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
#ifndef __FINDER_H__
#define __FINDER_H__

#include <string>
#include <vector>
#include "Line.h"
#include "SimpleChessboard.h"
#include "UCICommunicator.h"


//Forward decl
class UCIReceiver;

class Finder {
public:
    Finder(std::vector<int> &commIds);
    ~Finder();
    int runFinder();

protected:
    virtual int runFinderOnPosition(const Board::Position &pos,
                                    const std::list<std::string> &moves) = 0;
    /*Thread *startReceiver();*/
    static void sendPositionToEngine(Board::Position &pos, int commId);

    static Board::Color playFor_;

    /*Number of moves "played" by the finder*/
    int addedMoves_ = 0;

    /*Id of our communicators*/
    static std::vector<int> commIds_;

    static Comm::UCICommunicatorPool &pool_;

    /*Options instance*/
    static Options &opt_;

};

#endif
