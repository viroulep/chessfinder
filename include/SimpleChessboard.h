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
#ifndef __SIMPLECHESSBOARD_H__
#define __SIMPLECHESSBOARD_H__
#include <exception>
#include <vector>

#include "ChessboardTypes.h"

namespace Board {

    extern const std::string PieceToChar;

    class InvalidFenException : public std::exception {
        public:
            InvalidFenException(std::string msg);
            virtual const char* what() throw();
        private:
            std::string fenmsg;
    };

    class InvalidMoveException : public std::exception {
        public:
            InvalidMoveException(std::string msg);
            virtual const char* what() throw();
        private:
            std::string moveMsg;
    };

    typedef struct StateInfo {
        Square enpassant = SQ_NONE;
        int castle = 0, halfmoveClock = 0, fullmoveClock = 0;
        PieceKind captured = NO_KIND;
    } StateInfo;

    typedef struct Move {
        Square from = SQ_NONE;
        Square to = SQ_NONE;
        MoveType type = NO_TYPE;
        PieceKind promotion = NO_KIND;
        StateInfo *state = nullptr;
    } Move;

    class Position {
        public:
            Position();
            ~Position();
            Color side_to_move() const;
            bool empty(Square s) const;
            bool takes(Square attaker, Square target) const;
            Piece piece_on(Square s) const;
            Square enpassant() const;
            virtual void init();
            virtual void clear();
            void set(std::string fenString) throw(InvalidFenException);
            const std::string pretty() const;
            const std::string fen() const;
            //FIXME tmp debug
            Piece board_[64];
            void applyPseudoMove(Move m) throw(InvalidMoveException);
        protected:
            /*A board is an array of 64 pieces (can be NO_PIECE)*/
            Color active_;
            StateInfo startState_;
            StateInfo *st_ = nullptr;
            std::vector<Move> moves_;

            void setPos(std::string fenPos) throw(InvalidFenException);
            void setSide(std::string fenSide) throw(InvalidFenException);
            void setCastle(std::string fenCastle) throw(InvalidFenException);
            void setEP(std::string fenEP) throw(InvalidFenException);
            void setClock(std::string fenClock, int &clock) throw(InvalidFenException);
            void setFMClock(std::string fenFM) throw(InvalidFenException);

            inline bool can_castle(CastlingFlag f) const { return st_->castle & f; }
            inline bool can_castle(Color c) const
            {
                return st_->castle & ((W_OO | W_OOO) << (2 * c));
            }
    };


    inline Piece piece_from_char(char p)
    {
        size_t idx = PieceToChar.find(p);
        return (idx != std::string::npos)?Piece(idx):NO_PIECE;
    }

}

#endif
