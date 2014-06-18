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
#ifndef __MATFINDER_H__
#define __MATFINDER_H__

#include <string>
#include <vector>
#include "Line.h"
#include "SimpleChessboard.h"
#include "Finder.h"


class MatFinder : public Finder {
public:
    MatFinder(std::vector<int> &comm);
    virtual ~MatFinder();

private:
    int runFinderOnPosition(const Board::Position &pos,
                            const std::list<std::string> &moves);
    int computeMultiPV(const std::vector<Line> &lines);
    const Line &getBestLine(const Board::Position &pos,
                            const std::vector<Line> &lines);
    Line emptyLine_;
    int commId_;

};

#endif
