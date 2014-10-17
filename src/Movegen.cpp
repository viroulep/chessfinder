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
#include <iostream>
#include "Output.h"
#include "Movegen.h"

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
            Rank diff = Rank(abs(int(cr) - r));
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
        Color us = Color(color_of(pos.piece_on(from)));
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

#define GEN_ATTACKED(KIND) \
    template<>\
    set<Square> gen_attacked<KIND>(const Square from, const Position &pos)\
    {\
        return gen_reachable<KIND>(from, pos);\
    }

    GEN_ATTACKED(KNIGHT);
    GEN_ATTACKED(BISHOP);
    GEN_ATTACKED(ROOK);
    GEN_ATTACKED(QUEEN);
    GEN_ATTACKED(KING);

#undef GEN_ATTACKED

    template<>
    set<Square> gen_attacked<PAWN>(const Square from, const Position &pos)
    {
        set<Square> sqList;
        Rank r = rank_of(from);
        File f = file_of(from);
        Color us = Color(color_of(pos.piece_on(from)));
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
            DISPATCH(sqListTmp, kind_of(p), gen_attacked, s, pos);
            if (sqListTmp.find(target) != sqListTmp.end())
                sqList.insert(s);
        }
        return sqList;
    }
#define GEN_SIMPLE_MOVES(KIND) \
    template<>\
    vector<Move> gen_simple_moves<KIND>(const Square from, Position &pos)\
    {\
        set<Square> dests = gen_reachable<KIND>(from, pos);\
        vector<Move> moves;\
        for (Square s : dests) {\
            Move m;\
            m.from = from;\
            m.moving = pos.piece_on(from);\
            m.to = s;\
            m.type = NORMAL;\
            m.captured = NO_KIND;\
            if (!pos.empty(m.to))\
                m.captured = kind_of(pos.piece_on(m.to));\
            if (pos.tryMove(m))\
                moves.push_back(m);\
        }\
        return moves;\
    }

#define GEN_MOVES(KIND) \
    template<>\
    vector<Move> gen_moves<KIND>(const Square from, Position &pos)\
    {\
        return gen_simple_moves<KIND>(from, pos);\
    }

    GEN_SIMPLE_MOVES(QUEEN);
    GEN_SIMPLE_MOVES(KNIGHT);
    GEN_SIMPLE_MOVES(ROOK);
    GEN_SIMPLE_MOVES(BISHOP);
    GEN_SIMPLE_MOVES(KING);
    GEN_MOVES(QUEEN);
    GEN_MOVES(KNIGHT);
    GEN_MOVES(ROOK);
    GEN_MOVES(BISHOP);

#undef GEN_SIMPLE_MOVES
#undef GEN_MOVES

    template<>
    vector<Move> gen_moves<KING>(const Square from, Position &pos)
    {
        vector<Move> all = gen_simple_moves<KING>(from, pos);
        set<Square> attackers_oo;
        Piece king = pos.piece_on(from);
        Move m;
        m.from = from;
        m.moving = king;
        m.type = CASTLING;
        m.captured = NO_KIND;
        /*FIXME simplify (delegate castle checking to pos ?)*/
        /*NOTE target square legality is checked by tryMove*/
        if (color_of(king) == WHITE && from == SQ_E1) {
            if (pos.canCastle(W_OO) && pos.empty(SQ_F1)
                                    && pos.empty(SQ_G1)) {
                attackers_oo = gen_attackers(BLACK, SQ_F1, pos);
                if (attackers_oo.empty() && !pos.kingInCheck(WHITE)) {
                    m.to = SQ_G1;
                    if (pos.tryMove(m))
                        all.push_back(m);
                }
            }
            if (pos.canCastle(W_OOO) && pos.empty(SQ_D1)
                                     && pos.empty(SQ_C1)
                                     && pos.empty(SQ_B1)) {
                attackers_oo = gen_attackers(BLACK, SQ_D1, pos);
                if (attackers_oo.empty() && !pos.kingInCheck(WHITE)) {
                    m.to = SQ_C1;
                    if (pos.tryMove(m))
                        all.push_back(m);
                }
            }
        } else if (color_of(king) == BLACK && from == SQ_E8) {
            if (pos.canCastle(B_OO) && pos.empty(SQ_F8)
                                    && pos.empty(SQ_G8)) {
                attackers_oo = gen_attackers(WHITE, SQ_F8, pos);
                if (attackers_oo.empty() && !pos.kingInCheck(BLACK)) {
                    m.to = SQ_G8;
                    if (pos.tryMove(m))
                        all.push_back(m);
                }
            }
            if (pos.canCastle(B_OOO) && pos.empty(SQ_D8)
                                     && pos.empty(SQ_C8)
                                     && pos.empty(SQ_B8)) {
                attackers_oo = gen_attackers(WHITE, SQ_D8, pos);
                if (attackers_oo.empty() && !pos.kingInCheck(BLACK)) {
                    m.to = SQ_C8;
                    if (pos.tryMove(m))
                        all.push_back(m);
                }
            }
        }
        return all;
    }

    template<>
    vector<Move> gen_moves<PAWN>(const Square from, Position &pos)
    {
        vector<Move> all;
        set<Square> dests = gen_reachable<PAWN>(from, pos);
        Move m;
        m.from = from;
        m.moving = pos.piece_on(from);
        for (Square s : dests) {
            m.to = s;
            m.type = NORMAL;
            m.captured = NO_KIND;
            if (!pos.empty(m.to))
                m.captured = kind_of(pos.piece_on(m.to));
            if (front_or_back_rank(rank_of(s))) {
                /*Promotion*/
                m.type = PROMOTION;
                for (PieceKind k : promotion_kind()) {
                    m.promotion = k;
                    if (pos.tryMove(m))
                        all.push_back(m);
                }
            } else {
                if (pos.enpassant() == s) {
                    m.type = ENPASSANT;
                    m.captured = PAWN;
                }
                if (pos.tryMove(m))
                    all.push_back(m);
            }
        }
        return all;
    }

    vector<Move> gen_all(Position &pos)
    {
        vector<Move> all, partial;
        set<Square> squares = pos.pieces_squares(pos.side_to_move());
        Piece p;
        for (Square s : squares) {
            p = pos.piece_on(s);
            DISPATCH(partial, kind_of(p), gen_moves, s, pos);
            all.insert(all.end(), partial.begin(), partial.end());
        }
        return all;
    }

/*#undef DISPATCH*/
#undef MOVE_EP
#undef MOVE_LOOP
}
