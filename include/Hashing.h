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
#include <map>
#include <mutex>
#include <vector>

#include "ConcurrentMap.h"
#include "SimpleChessboard.h"

#define U64(u) (u##ULL)


class Node;

/*FIXME : is registering the next Node really necessary ?!*/
typedef std::pair<std::string, Node *> MoveNode;
typedef std::vector<MoveNode> LegalNodes;

class Node {
public:
    enum StatusFlag {
        PENDING = 1,
        AGAINST = 1 << 1,
        US = 1 << 2,
        THEM = 1 << 3,
        THRESHOLD = 1 << 4,
        MATE = 1 << 5,
        STALEMATE = 1 << 6,
        DRAW = 1 << 7,
        SIGNATURE_TABLE = 1 << 8
    };
    Node(const Node *prev);
    Node(const Node *prev, std::string pos, StatusFlag st);
    ~Node();
    void safeAddParent(const Node *parent);
    void safeAddMove(MoveNode mv);
    void updateStatus(StatusFlag st);
    void reset();
    const std::vector<const Node *> &getParents() const;
    const LegalNodes &getMoves() const;
    const std::string &getPos() const;
    StatusFlag getStatus() const;
    std::string to_string() const;
    static std::string to_string(StatusFlag s);
    /*"Light" copy : copy only basic informations such as status and moves,
     * as it's the only things needed for exporting table*/
    Node *lightCopy();
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
    StatusFlag st_ = PENDING;
    std::vector<const Node *> prev_;
    std::mutex lockP_;
    std::mutex lockM_;
    /*
     *bool lockP_ = false;
     *bool lockM_ = false;
     */
};

//map is internally ordered by key, ascending
class HashTable : public ConcurrentMap<uint64_t, Node *>
{
public:

    static uint64_t hashFEN(std::string fenString);
    HashTable(std::string file);
    HashTable();
    ~HashTable();
    std::string to_string();
    std::string show_pending();

    void autosave();
    void toPolyglot(const std::string &file);
    static HashTable *fromPolyglot(const std::string &file);
private:
    Node *unsafeFindPos(uint64_t hash);
    void outputHeader(std::ostream &os);
    void readHeader(std::istream &is);
    static int pieceOffset(int kind, Board::Rank r, Board::File f);
    static uint64_t piecesFromFEN(std::string pos);
    static uint64_t enpassantFromFEN(std::string enpassant);
    static uint64_t castleFromFEN(std::string castle);

    static const uint64_t Random64_[781];
    uint16_t cutoffValue_ = 0;
    const std::string file_;
};

#endif
