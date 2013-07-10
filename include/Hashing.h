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
#ifndef __HASHING_H__
#define __HASHING_H__

#include <cstdint>
#include "Chessboard.h"
#include "Board.h"

#ifdef _MSC_VER
#define U64(u) (u##ui64)
#else
#define U64(u) (u##ULL)
#endif

using namespace std;

class Hashing
{
public:
    static uint64_t hashFEN(string fenString);
    static uint64_t hashBoard(Chessboard *cb);

private:

    static int pieceOffset(int kind, Board::Rank r, Board::File f);
    static uint64_t turnFromFEN(string side);
    static uint64_t piecesFromFEN(string pos);

    static const uint64_t Random64_[781];
};
#endif
