#include "CompareMove.h"
#include "Utils.h"
#include "Output.h"



/*
 * Compare functions implements strict weak ordering (lhs < rhs)
 * ie: if a == b, compare(a,b) and compare(b,a) should both return false.
 */

using namespace Board;

MoveComparator::~MoveComparator()
{
}


/* m1 < m2 means that m1 is *better* than m2 ! */
bool MoveComparator::compare(const Position &pos, Move &lhs, Move &rhs)
{
    return evaluateMove(pos, lhs) > evaluateMove(pos, rhs);
}


uint16_t DefaultMoveComparator::evaluateMove(const Position &pos, Move &)
{
    return 1;
}

DefaultMoveComparator::~DefaultMoveComparator()
{
}

uint16_t MapMoveComparator::evaluateMove(const Position &pos, Move &mv)
{
    /*
     * Scheme of the evaluation :
     *
     * aabbfffrrrppp
     * aa   : value for take/promotion/pawn move
     * bb   : bonus for cuting the king
     * fff  : bonus for file
     * rrr  : bonus for rank
     * ppp  : bonus for piece kind
     **/
    File ft;
    Rank rt;
    uint16_t evaluation = 0x0;

    Square to = mv.to;
    Square from = mv.from;
    if (!is_ok(to) || !is_ok(from))
        Err::handle("Comparing move on undefined squares");
    ft = file_of(to);
    rt = rank_of(to);
    /*Piece *pf = from->getPiece();*/
    /*Piece *pt = to->getPiece();*/
    Piece pf = mv.moving;
    if (!from)
        Err::handle("No piece on from square");

    uint16_t encodedMove = 0x0;
    // promotion/Prise/ pion prioritaire
    if (mv.type == PROMOTION) {
        encodedMove = 0x3;
    /*} else if (mv.state->captured != NO_KIND) {*/
    } else if (mv.captured != NO_KIND) {
        encodedMove = 0x2;
    } else if (kind_of(pf) == PAWN) {
        encodedMove = 0x1;
    }

    uint16_t cutBonus = 0x0;
    if (kind_of(pf) == ROOK || kind_of(pf) == QUEEN) {
        Square oppKing = pos.king((pos.side_to_move() == WHITE)?BLACK:WHITE);
        if (int(rank_of(oppKing)) == int(rt) + 1)
            cutBonus = 0x1;
        cutBonus <<= 1;
        if (int(file_of(oppKing)) == int(ft) + 1)
            cutBonus  |= 0x1;
    }


    /*ppp*/
    uint16_t encodedPiece = 0x0;
    /*rrrfff*/
    uint16_t encodedSquare = 0x0;

    switch (kind_of(pf)) {
        case PAWN:
            encodedPiece = 0x1;
            break;
        case KNIGHT:
            /*Knight goes to C2*/
            encodedPiece = 0x2;
            encodedSquare = 7 - abs(int(rt) - RANK_2);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(int(ft) - FILE_C);
            break;
        case BISHOP:
            /*Bishop goes to D2*/
            encodedPiece = 0x3;
            encodedSquare = 7 - abs(int(rt) - RANK_2);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(int(ft) - FILE_D);
            break;
        case ROOK:
            /*Rook goes to B7 (Mainly 7th) */
            encodedPiece = 0x4;
            encodedSquare = 7 - abs(int(rt) - RANK_7);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(int(ft) - FILE_B);
            break;
        case QUEEN:
            /*Queen goes to C7 (Mainly 7th) */
            encodedPiece = 0x5;
            encodedSquare = 7 - abs(int(rt) - RANK_7);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(int(ft) - FILE_C);
            break;
        case KING:
            /*King goes to G4*/
            encodedPiece = 0x6;
            encodedSquare = 7 - abs(int(rt) - RANK_4);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(int(ft) - FILE_G);
            break;
        default:
            Err::handle("No kind to compare move !");
            break;
    }
    /*mmbbssssssppp*/
    evaluation = (encodedMove << 11) | (cutBonus << 9) | (encodedSquare << 3) | encodedPiece;
    return evaluation;
}

/*
uint16_t SampleMoveComparator::evaluateMove(Board::Move &mv)
{
    //Do some computation here
    return 1;
}
*/

