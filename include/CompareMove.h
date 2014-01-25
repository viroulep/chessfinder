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
#ifndef __COMPAREMOVE_H__
#define __COMPAREMOVE_H__

#include "Board.h"

class MoveComparator {
    public:
        virtual ~MoveComparator();
        bool compare(Board::Move &lhs, Board::Move &rhs);
        virtual uint16_t evaluateMove(Board::Move &mv) = 0;

};

class DefaultMoveComparator : public MoveComparator {
    public:
        virtual ~DefaultMoveComparator();
        uint16_t evaluateMove(Board::Move &mv);
};

class MapMoveComparator : public MoveComparator {
    public:
        uint16_t evaluateMove(Board::Move &mv);
};

namespace CompareMove {

bool compareTake(Board::Move &lhs, Board::Move &rhs);

}

#endif
