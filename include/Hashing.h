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
#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <cstdint>
#include <ostream>
#include <map>
#include <vector>

#include "SimpleChessboard.h"

#define U64(u) (u##ULL)


class Node;

typedef std::pair<std::string, Node *> MoveNode;
typedef std::vector<MoveNode> LegalNodes;

class Node {
public:
    enum Status {
        PENDING,
        AGAINST,
        TRESHOLD_US,
        TRESHOLD_THEM,
        MATE_US,
        MATE_THEM,
        STALEMATE,
        DRAW
    };
    Node(const Node *prev);
    Node(const Node *prev, std::string pos, Status st);
    ~Node();
    void safeAddParent(const Node *parent);
    void safeAddMove(MoveNode mv);
    void updateStatus(Status st);
    const std::vector<const Node *> &getParents() const;
    const LegalNodes &getMoves() const;
    const std::string &getPos() const;
    Status getStatus() const;
    std::string to_string() const;
    static std::string to_string(Status s);
private:
    /*Fen string without clock informations*/
    /*Actually its the full fen*/
    std::string pos_;
    /*
     * FIXME: only store the move !
     * And sort according to the natural ordering
     */
    //vector<UCIMove> legal_moves;
    LegalNodes legal_moves_;
    //Polyglot "learn" field, uint32_t
    Status st_ = PENDING;
    std::vector<const Node *> prev_;
    bool lockP_ = false;
    bool lockM_ = false;
};

//map is internally ordered by key, ascending
class HashTable : private std::multimap<uint64_t, Node *>
{
public:

    static uint64_t hashFEN(std::string fenString);
    /*static uint64_t hashBoard(Chessboard *cb);*/
    ~HashTable();
    std::string to_string();
    //TODO: rename simplepos to FEN
    Node *findPos(std::string sp);
    void safeAddNode(uint64_t hash, Node *node);
    int size() const;
    void toPolyglot(std::ostream &os);
    static HashTable *fromPolyglot(std::istream &is);
private:
    static int pieceOffset(int kind, Board::Rank r, Board::File f);
    static uint64_t piecesFromFEN(std::string pos);
    static uint64_t enpassantFromFEN(std::string enpassant);
    static uint64_t castleFromFEN(std::string castle);

    static const uint64_t Random64_[781];
    bool lock_ = false;
};

#endif
