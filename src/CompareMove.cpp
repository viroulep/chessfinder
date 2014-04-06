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
bool MoveComparator::compare(Board::Move &lhs, Board::Move &rhs)
{
    return evaluateMove(lhs) > evaluateMove(rhs);
}


bool CompareMove::compareTake(Move &lhs, Move &rhs)
{
    /*if (lhs.state->captured == NO_KIND && rhs.state->captured == NO_KIND)*/
    if (lhs.captured == NO_KIND && rhs.captured == NO_KIND)
        return false;
    /*else if (lhs.state->captured != NO_KIND && rhs.state->captured == NO_KIND)*/
    else if (lhs.captured != NO_KIND && rhs.captured == NO_KIND)
        return true;
    else if (lhs.captured == NO_KIND && rhs.captured != NO_KIND)
        return false;
    else {
        PieceKind lhsP = lhs.captured;
        PieceKind rhsP = rhs.captured;
        return lhsP > rhsP;
    }
}

uint16_t DefaultMoveComparator::evaluateMove(Move &)
{
    return 1;
}

DefaultMoveComparator::~DefaultMoveComparator()
{
}

uint16_t MapMoveComparator::evaluateMove(Move &mv)
{
    /*
     * Scheme of the evaluation :
     *
     * aafffrrrppp
     * aa : value for take/promotion/pawn move
     * fff : bonus for file
     * rrr : bonus for rank
     * ppp : bonus for piece kind
     **/
    File ft;
    Rank rt;
    uint16_t evaluation = 0x0;

    Square to = mv.to;
    Square from = mv.from;
    if (!to || !from)
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

    /*ppp*/
    uint16_t encodedPiece = 0x0;
    /*fffrrr*/
    uint16_t encodedSquare = 0x0;

    switch (kind_of(pf)) {
        case PAWN:
            encodedPiece = 0x1;
            break;
        case KNIGHT:
            /*Knight goes to C2*/
            encodedPiece = 0x2;
            encodedSquare = 7 - abs(ft - FILE_C);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(rt - RANK_2);
            break;
        case BISHOP:
            /*Bishop goes to D2*/
            encodedPiece = 0x3;
            encodedSquare = 7 - abs(ft - FILE_D);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(rt - RANK_2);
            break;
        case ROOK:
            /*Rook goes to B3*/
            encodedPiece = 0x4;
            encodedSquare = 7 - abs(ft - FILE_B);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(rt - RANK_3);
            break;
        case QUEEN:
            /*Queen goes to C3*/
            encodedPiece = 0x5;
            encodedSquare = 7 - abs(ft - FILE_C);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(rt - RANK_3);
            break;
        case KING:
            /*King goes to B2*/
            encodedPiece = 0x6;
            encodedSquare = 7 - abs(ft - FILE_B);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs(rt - RANK_2);
            break;
        default:
            Err::handle("No kind to compare move !");
            break;
    }
    /*mmssssssppp*/
    evaluation = (encodedMove << 9) | (encodedSquare << 3) | encodedPiece;
    return evaluation;
}

/*
uint16_t SampleMoveComparator::evaluateMove(Board::Move &mv)
{
    //Do some computation here
    return 1;
}
*/

