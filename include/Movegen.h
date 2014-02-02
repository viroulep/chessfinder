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
#ifndef __MOVEGEN_H__
#define __MOVEGEN_H__

#include "ChessboardTypes.h"
#include "SimpleChessboard.h"
#include <set>

namespace Board {

    /*
     * Generates the list of all the reachable squares for a piece from a
     * square. A square is reachable if the piece can move to the empty square,
     * or if the piece on destination square is from opposite color.
     * Assumes there IS a piece on 'from' square !
     * Does NOT check if the destination square is legal ! (A king can 'reach'
     * a square attacked by an opposite piece)
     * */
    template<PieceKind K>
        std::set<Square> gen_reachable(const Square from, const Position &pos);
    /*
     * Generates the list of all the attacked squares from a piece on a
     * square.
     * Usually these are the reachable squares, except for pawns...
     * Assumes there IS a piece on 'from' square !
     * */
    template<PieceKind K>
        std::set<Square> gen_attacked(const Square from, const Position &pos);

    /*
     * Generates the list of squares where there is a piece of color c attacking
     * the 'target'.
     */
    std::set<Square> gen_attacker(Color c, const Square target, const Position &pos);

    /*
     * Generates the list of all moves for the given piece
     * Assumes there IS a piece on 'from' square !
     * */
    /*TODO actual move gen, with castling, and pawn promotion*/
    template<PieceKind K>
        std::set<Move> gen_moves(const Square from, const Position &pos);




}

#endif
