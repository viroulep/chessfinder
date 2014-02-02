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
#include <fstream>
#include <cstdint>
#include "SimpleChessboard.h"
#include "Movegen.h"
/*#include "Hashing.h"*/
/*#include "Utils.h"*/
/*#include "Options.h"*/

using namespace std;
using namespace Board;


#define DISPLAY_SET(setName) \
cout << "Display :";\
for (Square s : setName) {\
    cout << " " << square_to_string(s);\
}\
cout << "\n";

#define DISPLAY_MOVES(vectorName) \
cout << "Display moves :";\
for (Move m : vectorName) {\
    cout << " (" << square_to_string(m.from)\
    << square_to_string(m.to) << "," << movetype_to_string(m.type) << ")";\
}\
cout << "\n";

#define TEST_ATTACK(target, color, setName) \
    setName = Board::gen_attacker(color, target, pos);\
cout << "Attacking " << square_to_string(target) << " as "\
<< color_to_string(color) << "\n";\
DISPLAY_SET(setName);

#define TEST_MOVE(rank, file, piece, setName) \
    pos.board_[make_square(rank, file)] = piece;\
switch (kind_of(piece)) {\
    case KING:\
        setName = Board::gen_reachable<KING>(make_square(rank, file), pos);\
    break;\
    case QUEEN:\
        setName = Board::gen_reachable<QUEEN>(make_square(rank, file), pos);\
    break;\
    case BISHOP:\
        setName = Board::gen_reachable<BISHOP>(make_square(rank, file), pos);\
    break;\
    case KNIGHT:\
        setName = Board::gen_reachable<KNIGHT>(make_square(rank, file), pos);\
    break;\
    case ROOK:\
        setName = Board::gen_reachable<ROOK>(make_square(rank, file), pos);\
    break;\
    case PAWN:\
        setName = Board::gen_reachable<PAWN>(make_square(rank, file), pos);\
    break;\
    default:\
            break;\
}\
cout << "Reachable :";\
for (Square s : setName) {\
    cout << " " << square_to_string(s);\
    pos.board_[s] = piece;\
}\
cout << "\n";\
cout << pos.pretty() << endl;\
pos.set(startFen);

#define APPLY(mVar, fromSQ, toSQ, moveType) \
    mVar.from = fromSQ;\
mVar.to = toSQ;\
mVar.type = moveType;\
pos.applyMove(mVar);

#define TEST_GENMOVE(type, from, vector) \
    vector = Board::gen_moves<type>(from, pos);


int main()
{
    cout << "Sizeof struct : " << sizeof(Square) << "\n";
    cout << "b2 ? :" << to_string(SQ_B4 & 7) << ":" << to_string(SQ_B4 >> 3) << "\n";
    string startFen = "8/8/7k/1Q2P3/7K/q7/8/1R4b1 w - f6 0 1";
    Position pos;
    pos.init();
    /*pos.set(startFen);*/
    cout << pos.pretty() << endl;

    set<Square> gen;
    vector<Move> moves;

    Move m;
    try {
        APPLY(m, SQ_E2, SQ_E4, NORMAL);
        cout << pos.pretty() << endl;
        APPLY(m, SQ_F7, SQ_F5, NORMAL);
        cout << pos.pretty() << endl;
        APPLY(m, SQ_E4, SQ_E5, NORMAL);
        cout << pos.pretty() << endl;
        APPLY(m, SQ_D7, SQ_D5, NORMAL);
        cout << pos.pretty() << endl;
        APPLY(m, SQ_F1, SQ_B5, NORMAL);
        cout << pos.pretty() << endl;
        TEST_GENMOVE(QUEEN, SQ_D8, moves);
        DISPLAY_MOVES(moves);
        APPLY(m, SQ_D8, SQ_D7, NORMAL);
        cout << pos.pretty() << endl;
        /*
         *APPLY(m, SQ_E7, SQ_E6, NORMAL);
         *cout << pos.pretty() << endl;
         *APPLY(m, SQ_G1, SQ_F3, NORMAL);
         *cout << pos.pretty() << endl;
         *APPLY(m, SQ_G8, SQ_F6, NORMAL);
         *cout << pos.pretty() << endl;
         *APPLY(m, SQ_E1, SQ_G1, CASTLING);
         *cout << pos.pretty() << endl;
         *APPLY(m, SQ_D5, SQ_C4, NORMAL);
         *cout << pos.pretty() << endl;
         *pos.undoMove();
         *cout << pos.pretty() << endl;
         *pos.undoMove();
         *cout << pos.pretty() << endl;
         *pos.undoMove();
         *cout << pos.pretty() << endl;
         *pos.undoMove();
         *cout << pos.pretty() << endl;
         *pos.undoMove();
         *cout << pos.pretty() << endl;
         */
        pos.undoMove();
        cout << pos.pretty() << endl;
        pos.undoMove();
        cout << pos.pretty() << endl;
        pos.undoMove();
        cout << pos.pretty() << endl;
        pos.undoMove();
        cout << pos.pretty() << endl;
        pos.undoMove();
        cout << pos.pretty() << endl;
        pos.undoMove();
        cout << pos.pretty() << endl;
    } catch (InvalidMoveException e) {
        cerr << "Exception : " << e.what() << "\n";
    }

/*
 *    cout << "Knight :\n";
 *    TEST_MOVE(RANK_6, FILE_B, W_KNIGHT, gen);
 *
 *    cout << "Bishop :\n";
 *    TEST_MOVE(RANK_1, FILE_A, W_BISHOP, gen);
 *    TEST_MOVE(RANK_2, FILE_B, W_BISHOP, gen);
 *    TEST_MOVE(RANK_5, FILE_C, W_BISHOP, gen);
 *
 *    cout << "Rook :\n";
 *    TEST_MOVE(RANK_1, FILE_A, W_ROOK, gen);
 *    TEST_MOVE(RANK_2, FILE_B, W_ROOK, gen);
 *    TEST_MOVE(RANK_5, FILE_C, W_ROOK, gen);
 *    TEST_MOVE(RANK_3, FILE_D, B_ROOK, gen);
 *
 *    cout << "Queen :\n";
 *    TEST_MOVE(RANK_1, FILE_A, W_QUEEN, gen);
 *    TEST_MOVE(RANK_2, FILE_B, W_QUEEN, gen);
 *    TEST_MOVE(RANK_5, FILE_C, W_QUEEN, gen);
 *    TEST_MOVE(RANK_3, FILE_A, B_QUEEN, gen);
 *    TEST_MOVE(RANK_3, FILE_D, B_QUEEN, gen);
 *
 *    cout << "King :\n";
 *    TEST_MOVE(RANK_1, FILE_A, W_KING, gen);
 *    TEST_MOVE(RANK_2, FILE_B, W_KING, gen);
 *    TEST_MOVE(RANK_5, FILE_C, W_KING, gen);
 *
 *    cout << "Pawn :\n";
 *    TEST_MOVE(RANK_7, FILE_F, W_PAWN, gen);
 *    TEST_MOVE(RANK_5, FILE_H, W_PAWN, gen);
 *    TEST_MOVE(RANK_2, FILE_G, W_PAWN, gen);
 *    TEST_MOVE(RANK_2, FILE_H, W_PAWN, gen);
 *    TEST_MOVE(RANK_2, FILE_B, W_PAWN, gen);
 *    TEST_MOVE(RANK_6, FILE_C, B_PAWN, gen);
 *
 *    cout << pos.pretty();
 *    cout << "Test attack :\n";
 *    TEST_ATTACK(make_square(RANK_1, FILE_G), WHITE, gen);
 *    TEST_ATTACK(make_square(RANK_1, FILE_F), WHITE, gen);
 *    TEST_ATTACK(make_square(RANK_4, FILE_D), WHITE, gen);
 *    TEST_ATTACK(make_square(RANK_4, FILE_D), BLACK, gen);
 *    TEST_ATTACK(make_square(RANK_5, FILE_C), WHITE, gen);
 *    TEST_ATTACK(make_square(RANK_5, FILE_C), BLACK, gen);
 */


#if 0
    string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos.set(startFen);
    cout << pos.pretty() << endl;
    cout << "import : " << startFen << "\n";
    cout << "export : " << pos.fen() << "\n";

    string startFen2 = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2";
    pos.set(startFen2);
    cout << pos.pretty();
    cout << "import : " << startFen2 << "\n";
    cout << "export : " << pos.fen() << "\n";

    string fen = "2r5/3P4/8/8/5K2/8/3k4/8 w - - 0 38";
    pos.set(fen);
    cout << pos.pretty();
    cout << "import : " << fen << "\n";
    cout << "export : " << pos.fen() << "\n";
    set<Square> gen = Board::gen_reachable<KNIGHT>(make_square(RANK_6, FILE_B), pos);
    cout << "Reachable :";
    for (Square s : gen)
        cout << " " << square_to_string(s);
    cout << "\n";
    gen = Board::gen_reachable<KNIGHT>(make_square(RANK_4, FILE_E), pos);
    cout << "Reachable :";
    for (Square s : gen)
        cout << " " << square_to_string(s);
    cout << "\n";


    Options::setVerboseLevel(2);
    //string fen = "rnbqkbnr/pp2pppp/8/2ppP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3";
    /*string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";*/
    string startFen = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2";
    /*string startFen = "rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3";*/
    /*string startFen = "rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4";*/
    /*uint64_t expected = 0x823c9b50fd114196;*/
    uint64_t expected = 0x0756b94461c50fb0;
    uint64_t display  = 0xCF3145DE0ADD4289;
    string fenfen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
    string fen = "2r5/3P4/8/8/5K2/8/3k4/8 w - - 0 38";
    string fenInsuficient = "8/8/8/3k4/3b4/3K4/8/8 w - - 0 1";
    //Chessboard *test = Chessboard::createChessboard();
    Chessboard *test = Chessboard::createFromFEN(startFen);
    /*uint64_t hash = HashTable::hashBoard(test);*/
    uint64_t hashFen = HashTable::hashFEN(startFen);

    Utils::output(test->to_string() + "\n");
    Utils::output("Sufficient  : " + to_string(test->sufficientMaterial())
            + "\n");
    Utils::output("Hashs : \n");
    /*Utils::output("Board : " + to_string(hash) + "\n");*/
    Utils::output("FEN   : " + to_string(hashFen) + "\n");
    Utils::output("Expected   : " + to_string(expected) + "\n");
    Utils::output("Display   : " + to_string(display) + "\n");
    Utils::output("Fens : \n");
    Utils::output("Orig  : " + startFen + "\n");
    Utils::output("Board : " + test->exportToFEN() + "\n");
/*
 *    test->reInitFromFEN(startFen);
 *    Utils::output(test->to_string() + "\n");
 *    Utils::output("Sufficient  : " + to_string(test->sufficientMaterial())
 *            + "\n");
 *    Utils::output("Fens : \n");
 *    Utils::output("Orig  : " + startFen + "\n");
 *    Utils::output("Board : " + test->exportToFEN() + "\n");
 *
 *    UCIMove mv = "e2e4n";
 *    Utils::output("e2e4n : " + to_string(Board::uciToPolyglot(mv)) + "\n");
 *    Utils::output("4892 : " + Board::polyglotToUci(4892) + "\n");
 *    Utils::output("\n");
 *
 *    HashTable *tableTest = new HashTable();
 *    Node *node = new Node();
 *    node->pos = fen;
 *    MoveNode mn;
 *    mn.first = "f4f5";
 *    mn.second = NULL;
 *    node->legal_moves.push_back(mn);
 *    node->st = Node::DRAW;
 *    pair<uint64_t, Node *> p(HashTable::hashFEN(fen), node);
 *    tableTest->insert(p);
 *
 *    Node *node2 = new Node();
 *    node2->pos = fen;
 *    mn.first = "e2e4";
 *    mn.second = NULL;
 *    node2->legal_moves.push_back(mn);
 *    node2->st = Node::MATE;
 *    pair<uint64_t, Node *> p2(HashTable::hashFEN(fenfen), node2);
 *    tableTest->insert(p2);
 *
 *    Utils::output(tableTest->to_string() + "\n");
 *
 *    Utils::output("Ouput to file\n");
 *    ofstream outputFile("oracleGardner.bin", ios::binary);
 *    tableTest->toPolyglot(outputFile);
 *    outputFile.close();
 *
 *    test->reInitFromFEN(fenInsuficient);
 *    Utils::output(test->to_string() + "\n");
 *    Utils::output("Sufficient  : " + to_string(test->sufficientMaterial())
 *            + "\n");
 *
 *
 *    delete tableTest;
 *
 *
 *    Utils::output("Input from file\n");
 *    ifstream inputFile("oracleGardner.bin", ios::binary);
 *    HashTable *other = HashTable::fromPolyglot(inputFile);
 *    Utils::output("Table :\n");
 *    Utils::output(other->to_string() + "\n");
 *    delete other;
 *    inputFile.close();
 */


    /*
     *test->uciApplyMove("e2e4");
     *test->uciApplyMove("e7e6");
     *test->uciApplyMove("d2d4");
     *test->uciApplyMove("d7d5");
     *test->uciApplyMove("e4d5");
     *test->uciApplyMove("e6d5");
     *test->uciApplyMove("g1f3");
     *test->uciApplyMove("g8f6");
     *test->uciApplyMove("f1e2");
     *test->uciApplyMove("c8e6");
     *test->uciApplyMove("e1g1");
     *test->uciApplyMove("b8c6");
     *test->uciApplyMove("f1e1");
     *Utils::output(test->to_string() + "\n");
     *const list<string> uciMoves = test->getUciMoves();
     *for (list<string>::const_iterator it = uciMoves.begin(), itEnd = uciMoves.end();
     *        it != itEnd; ++it)
     *    Utils::output((*it) + " ");
     *Utils::output("\n");
     */

    //test->uciApplyMove("f4e4");
    //test->uciApplyMove("d2c3");
    //test->uciApplyMove("d7d8q");
    //test->uciApplyMove("c8d8");
    //Utils::output(test->to_string() + "\n");
    //test->uciApplyMove("d2d3");
    //test->uciApplyMove("f4f5");
    //Utils::output(test->to_string() + "\n");

    /*
     *test->uciApplyMove("e2e4");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("c7c5");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("g1f3");
     *Utils::output(test->to_string() + "\n");
     */

    /*
     *list<string> movesToTry;
     *movesToTry.push_back("g1f3");
     *movesToTry.push_back("d7d6");
     *movesToTry.push_back("d2d4");
     *movesToTry.push_back("c5d4");
     *Utils::output("Trying a line :\n");
     */
    //Utils::output(test->tryUciMoves(movesToTry) + "\n");
    //Utils::output(test->to_string() + "\n");

    /*
     *test->uciApplyMove("d7d6");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("d2d4");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("c5d4");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("f3d4");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("g8f6");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("b1c3");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("g7g6");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("c1e3");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("f8g7");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("f2f3");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("e8g8");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("d1d2");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("b8c6");
     *Utils::output(test->to_string() + "\n");
     *test->uciApplyMove("e1c1");
     *Utils::output(test->to_string() + "\n");
     *const list<string> uciMoves = test->getUciMoves();
     *for (list<string>::const_iterator it = uciMoves.begin(), itEnd = uciMoves.end();
     *        it != itEnd; ++it)
     *    Utils::output((*it) + " ");
     *Utils::output("\n");
     */

    /*
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     *test->undoMove();
     *Utils::output(test->to_string() + "\n");
     */
    //Utils::output(test->to_string() + "\n");
    //Utils::output(test->to_string() + "\n");
    delete test;
    return 0;
#endif
}
