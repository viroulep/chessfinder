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
#include "Movegen.h"
#include <iostream>

using namespace std;

namespace Board {

#define MOVE_LOOP(cond, rank, file) \
    for (cond) {                \
        Square s = make_square(rank, file);\
        if (pos.empty(s) || pos.takes(from, s))\
        sqList.insert(s);\
        if (!pos.empty(s))\
        break;\
    }

#define MOVE_EP(s, active, rank, file)\
    s = (active == WHITE)?\
    make_square(Rank(rank + 1), file):\
    make_square(Rank(rank - 1), file);\
    if (is_ok(s) && (s == pos.enpassant() || pos.takes(from, s)))\
    sqList.insert(s);

    template<>
    set<Square> gen_reachable<KNIGHT>(const Square from, const Position &pos)
    {
        set<Square> sqList;
        Rank r = rank_of(from);
        File f = file_of(from);
        for (Rank cr = Rank(r - 2); cr <= Rank(r + 2); ++cr) {
            if (cr == r)
                continue;
            Rank diff = Rank(abs(cr - r));
            File fdiff = File(3 - diff);
            Square s1 = make_square(cr, File(f + fdiff));
            Square s2 = make_square(cr, File(f - fdiff));
            if (is_ok(s1) && (pos.empty(s1) || pos.takes(from, s1)))
                sqList.insert(s1);
            if (is_ok(s2) && (pos.empty(s2) || pos.takes(from, s2)))
                sqList.insert(s2);
        }
        return sqList;
    }

    template<>
    set<Square> gen_reachable<ROOK>(const Square from, const Position &pos)
    {
        set<Square> sqList;
        Rank r = rank_of(from);
        File f = file_of(from);

        MOVE_LOOP(File cf = File(f + 1); is_ok(make_square(r, cf)); ++cf, r, cf);

        MOVE_LOOP(File cf = File(f - 1); is_ok(make_square(r, cf)); --cf, r, cf);

        MOVE_LOOP(Rank cr = Rank(r - 1); is_ok(make_square(cr, f)); --cr, cr, f);

        MOVE_LOOP(Rank cr = Rank(r + 1); is_ok(make_square(cr, f)); ++cr, cr, f);

        return sqList;
    }

    template<>
    set<Square> gen_reachable<BISHOP>(const Square from, const Position &pos)
    {
        set<Square> sqList;
        Rank r = rank_of(from);
        File f = file_of(from);
        File cf;
        Rank cr;
        MOVE_LOOP((cf = File(f + 1), cr = Rank(r + 1));
                is_ok(make_square(cr, cf)); (++cf,++cr), cr, cf);

        MOVE_LOOP((cf = File(f + 1), cr = Rank(r - 1));
                is_ok(make_square(cr, cf)); (++cf,--cr), cr, cf);

        MOVE_LOOP((cf = File(f - 1), cr = Rank(r - 1));
                is_ok(make_square(cr, cf)); (--cf,--cr), cr, cf);

        MOVE_LOOP((cf = File(f - 1), cr = Rank(r + 1));
                is_ok(make_square(cr, cf)); (--cf,++cr), cr, cf);

        return sqList;
    }

    template<>
    set<Square> gen_reachable<QUEEN>(const Square from, const Position &pos)
    {
        set<Square> sqList = gen_reachable<BISHOP>(from, pos);
        set<Square> sqListR = gen_reachable<ROOK>(from, pos);
        sqList.insert(sqListR.begin(), sqListR.end());

        return sqList;
    }

    template<>
    set<Square> gen_reachable<KING>(const Square from, const Position &pos)
    {
        set<Square> sqList;
        Rank r = rank_of(from);
        File f = file_of(from);
        for (File cf = File(f - 1); cf <= File(f + 1); ++cf)
            for (Rank cr = Rank(r - 1); cr <= Rank(r + 1); ++cr) {
                Square s = make_square(cr, cf);
                if (!is_ok(s) || s == from)
                    continue;
                if (pos.empty(s) || pos.takes(from, s))
                    sqList.insert(s);
            }
        return sqList;
    }

    template<>
    set<Square> gen_reachable<PAWN>(const Square from, const Position &pos)
    {
        set<Square> sqList;
        Rank r = rank_of(from);
        File f = file_of(from);
        Color us = Color(pos.piece_on(from));
        Square dest;
        MOVE_EP(dest, us, r, File(f + 1));
        MOVE_EP(dest, us, r, File(f - 1));

        Rank rp = (us == WHITE)?Rank(r + 1):Rank(r - 1);

        dest = make_square(rp, f);
        if (is_ok(dest) && pos.empty(dest)) {
            sqList.insert(dest);
            Rank startRank = (us == WHITE)?RANK_2:RANK_7;
            if (r == startRank) {
                Rank rdp = (us == WHITE)?Rank(r + 2):Rank(r - 2);
                dest = make_square(rdp, f);
                if (is_ok(dest) && pos.empty(dest))
                    sqList.insert(dest);
            }
        }
        return sqList;
    }

    template<>
    set<Square> gen_attacked<KNIGHT>(const Square from, const Position &pos)
    {
        return gen_reachable<KNIGHT>(from, pos);
    }

    template<>
    set<Square> gen_attacked<BISHOP>(const Square from, const Position &pos)
    {
        return gen_reachable<BISHOP>(from, pos);
    }

    template<>
    set<Square> gen_attacked<ROOK>(const Square from, const Position &pos)
    {
        return gen_reachable<ROOK>(from, pos);
    }

    template<>
    set<Square> gen_attacked<QUEEN>(const Square from, const Position &pos)
    {
        return gen_reachable<QUEEN>(from, pos);
    }

    template<>
    set<Square> gen_attacked<KING>(const Square from, const Position &pos)
    {
        /*TODO : think if it's needed to also add the squares with a 'friend'
         * piece !
         * */
        return gen_reachable<KING>(from, pos);
    }

    template<>
    set<Square> gen_attacked<PAWN>(const Square from, const Position &pos)
    {
        set<Square> sqList;
        Rank r = rank_of(from);
        File f = file_of(from);
        Color us = pos.side_to_move();
        Rank ra = (us == WHITE)?Rank(r + 1):Rank(r - 1);
        Square dest;
        dest = make_square(ra, File(f + 1));
        if (is_ok(dest))
            sqList.insert(dest);
        dest = make_square(ra, File(f - 1));
        if (is_ok(dest))
            sqList.insert(dest);

        return sqList;
    }


    set<Square> gen_attackers(Color c, const Square target, const Position &pos)
    {
        set<Square> sqList;
        for (Square s = SQ_A1; s <= SQ_H8; ++s) {
            set<Square> sqListTmp;
            Piece p = pos.piece_on(s);
            if (p == NO_PIECE || color_of(p) != c)
                continue;
            switch (kind_of(p)) {
                case KNIGHT:
                    sqListTmp = gen_attacked<KNIGHT>(s, pos);
                    break;
                case BISHOP:
                    sqListTmp = gen_attacked<BISHOP>(s, pos);
                    break;
                case ROOK:
                    sqListTmp = gen_attacked<ROOK>(s, pos);
                    break;
                case QUEEN:
                    sqListTmp = gen_attacked<QUEEN>(s, pos);
                    break;
                case KING:
                    sqListTmp = gen_attacked<KING>(s, pos);
                    break;
                case PAWN:
                    sqListTmp = gen_attacked<PAWN>(s, pos);
                    break;
                default:
                    break;
            }
            if (sqListTmp.find(target) != sqListTmp.end())
                sqList.insert(s);
        }
        return sqList;
    }

    template<>
    vector<Move> gen_moves<QUEEN>(const Square from, Position &pos)
    {
        set<Square> dests = gen_reachable<QUEEN>(from, pos);
        vector<Move> moves;
        for (Square s : dests) {
            Move m;
            m.from = from;
            m.to = s;
            m.type = NORMAL;
            try {
                pos.applyMove(m);
                pos.undoMove();
                moves.push_back(m);
            } catch (InvalidMoveException e) {
                continue;
            }
        }
        return moves;
    }


#undef MOVE_EP
#undef MOVE_LOOP
}
