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
#ifndef __CHESSBOARDTYPES_H__
#define __CHESSBOARDTYPES_H__

#include <cstdint>
#include <string>
#include <assert.h>

namespace Board {

    enum Color {
        WHITE,
        BLACK,
        NOCOLOR
    };

    enum MoveType {
        NO_TYPE,
        NORMAL,
        PROMOTION,
        ENPASSANT,
        CASTLING
    };

    enum CastlingFlag {
        NONE  = 0x0,
        W_OO  = 0x0001,
        W_OOO = 0x0010,
        B_OO  = 0x0100,
        B_OOO = 0x1000
    };

    enum PieceKind {
        NO_KIND,
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING
    };

    enum Piece {
        NO_PIECE,
        W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
        B_PAWN = 9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
    };

    enum Rank {
        RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
    };

    enum File {
        FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
    };

    enum Square {
        /*=Rank * 8 + File*/
        SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
        SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
        SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
        SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
        SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
        SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
        SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
        SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
        SQ_NONE
    };

#define ENABLE_SAFE_OPERATORS_ON(T)                                            \
    inline T operator+(const T d1, const T d2) { return T(int(d1) + int(d2)); }\
    inline T operator-(const T d1, const T d2) { return T(int(d1) - int(d2)); }\
    inline T operator*(int i, const T d) { return T(i * int(d)); }             \
    inline T operator*(const T d, int i) { return T(int(d) * i); }             \
    inline T operator-(const T d) { return T(-int(d)); }                       \
    inline T& operator+=(T& d1, const T d2) { return d1 = d1 + d2; }           \
    inline T& operator-=(T& d1, const T d2) { return d1 = d1 - d2; }           \
    inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }


#define ENABLE_OPERATORS_ON(T) ENABLE_SAFE_OPERATORS_ON(T)        \
    inline T& operator++(T& d) { return d = T(int(d) + 1); }      \
    inline T& operator--(T& d) { return d = T(int(d) - 1); }      \
    inline T operator/(const T d, int i) { return T(int(d) / i); }\
    inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

    ENABLE_OPERATORS_ON(File)
    ENABLE_OPERATORS_ON(Rank)
    ENABLE_OPERATORS_ON(Square)
    ENABLE_OPERATORS_ON(PieceKind)

#undef ENABLE_OPERATORS_ON
#undef ENABLE_SAFE_OPERATORS_ON

    inline Piece make_piece(Color c, PieceKind pt) {
        return Piece((c << 3) | pt);
    }

    inline Square make_square(Rank r, File f) {
        return (r < RANK_1 || r > RANK_8 || f < FILE_A || f > FILE_H)?
            SQ_NONE:Square((r << 3) | f);
    }

    /*
     *inline CastlingFlag make_castling_flag(Color c, CastlingSide s) {
     *    return CastlingFlag(WHITE_OO << ((s == QUEEN_SIDE) + 2 * c));
     *}
     */

    inline PieceKind kind_of(Piece p) {
        return PieceKind(p & 7);
    }

    inline Color color_of(Piece p) {
        assert(p != NO_PIECE);
        return Color(p >> 3);
    }

    inline bool is_ok(Square s) {
        return s >= SQ_A1 && s <= SQ_H8;
    }

    inline File file_of(Square s) {
        return File(s & 7);
    }

    inline Rank rank_of(Square s) {
        return Rank(s >> 3);
    }

    inline bool front_or_back_rank(Rank r)
    {
        return (r == RANK_1 || r == RANK_8);
    }

    /*
     *    inline Square relative_square(Color c, Square s) {
     *        return Square(s ^ (c * 56));
     *    }
     *
     *    inline Rank relative_rank(Color c, Rank r) {
     *        return Rank(r ^ (c * 7));
     *    }
     *
     *    inline Rank relative_rank(Color c, Square s) {
     *        return relative_rank(c, rank_of(s));
     *    }
     */

    inline bool opposite_colors(Square s1, Square s2) {
        int s = int(s1) ^ int(s2);
        return ((s >> 3) ^ s) & 1;
    }

    inline char file_to_char(File f, bool tolower = true) {
        return char(f - FILE_A + (tolower ? 'a' : 'A'));
    }

    inline char rank_to_char(Rank r) {
        return char(r - RANK_1 + '1');
    }

    inline char kind_to_char(PieceKind k, bool printPawn = true,
            bool toLower = false)
    {
        assert(k != NO_KIND);
        char c = '#';
        switch (k) {
            case PAWN:
                c = (printPawn)?((toLower)?'p':'P'):'\0';
                break;
            case KNIGHT:
                c = (toLower?'n':'N');
                break;
            case BISHOP:
                c = (toLower?'b':'B');
                break;
            case ROOK:
                c = (toLower?'r':'R');
                break;
            case QUEEN:
                c = (toLower?'q':'Q');
                break;
            case KING:
                c = (toLower?'k':'K');
                break;
            default:
                assert(false);
                break;
        }
        return c;
    }

    inline Square square_from_string(std::string sq)
    {
        Square s = SQ_NONE;
        if (sq.size() == 2) {
            char f,r;
            f = sq[0];
            r = sq[1];
            if (f >= 'a' && f <= 'h' && r >= '1' && r <= '8')
                s = make_square(Rank(r - '1'), File(f - 'a'));
        }
        return s;
    }

    /*
     *inline Square pawn_push(Color c) {
     *    return c == WHITE ? DELTA_N : DELTA_S;
     *}
     */

    /*
     *    inline Square from_sq(Move m) {
     *        return Square((m >> 6) & 0x3F);
     *    }
     *
     *    inline Square to_sq(Move m) {
     *        return Square(m & 0x3F);
     *    }
     */

    /*
     *    inline MoveType type_of(Move m) {
     *        return MoveType(m & (3 << 14));
     *    }
     *
     *    inline PieceType promotion_type(Move m) {
     *        return PieceType(((m >> 12) & 3) + 2);
     *    }
     *
     *    inline Move make_move(Square from, Square to) {
     *        return Move(to | (from << 6));
     *    }
     *
     *    template<MoveType T>
     *        inline Move make(Square from, Square to, PieceType pt = KNIGHT) {
     *            return Move(to | (from << 6) | T | ((pt - KNIGHT) << 12));
     *        }
     *
     *    inline bool is_ok(Move m) {
     *        return from_sq(m) != to_sq(m); // Catches also MOVE_NULL and MOVE_NONE
     *    }
     */

    inline const std::string square_to_string(Square s)
    {
        char ch[] = { file_to_char(file_of(s)), rank_to_char(rank_of(s)), 0 };
        return ch;
    }

    inline const std::string piece_to_string(Piece p, bool withColor = false)
    {
        const std::string RED = "\e[31;1m";
        const std::string RESET = "\e[0m";
        std::string retVal;
        if (kind_of(p) != NO_KIND) {
            char ch[] = {kind_to_char(kind_of(p)) , 0};
            retVal = ch;
            if (color_of(p) == BLACK)
                retVal = RED + retVal + RESET;
        } else
            retVal = " ";
        return retVal;
    }

    inline const std::string color_to_string(Color c, bool withColor = false)
    {
        const std::string RED = "\e[31;1m";
        const std::string RESET = "\e[0m";
        std::string retVal;
        assert(c != NOCOLOR);
        retVal = (c == WHITE)?"White":"Black";
        if (withColor && c == BLACK)
            retVal = RED + retVal + RESET;
        return retVal;
    }

    inline const std::string movetype_to_string(MoveType t)
    {
        std::string retVal;
        switch (t) {
            case NORMAL:
                retVal = "normal";
                break;
            case CASTLING:
                retVal = "castle";
                break;
            case PROMOTION:
                retVal = "promotion";
                break;
            case ENPASSANT:
                retVal = "ep";
                break;
            default:
                retVal = "unknown";
        }
        return retVal;
    }

    //ALAMOS
    inline bool is_in_alamos(Square s) {
        return (!(rank_of(s) == RANK_8 ||
                    rank_of(s) == RANK_1 ||
                    file_of(s) == FILE_A ||
                    file_of(s) == FILE_H));
    }
}

#endif
