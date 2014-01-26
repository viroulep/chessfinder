#include "CompareMove.h"
#include "Utils.h"



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
    if (!lhs.takePiece && !rhs.takePiece)
        return false;
    else if (lhs.takePiece)
        return true;
    else if (rhs.takePiece)
        return false;
    else {
        Piece *lhsP = lhs.to->getPiece();
        Piece *rhsP = rhs.to->getPiece();
        return lhsP->getKind() < rhsP->getKind();
    }
}

uint16_t DefaultMoveComparator::evaluateMove(Move &mv)
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

    Square *to = mv.to;
    Square *from = mv.from;
    if (!to || !from)
        Utils::handleError("Comparing move on undefined squares");
    ft = to->getFile();
    rt = to->getRank();
    Piece *pf = from->getPiece();
    Piece *pt = to->getPiece();
    if (!from)
        Utils::handleError("No piece on from square");

    uint16_t encodedMove = 0x0;
    // promotion/Prise/ pion prioritaire
    if (mv.promoteTo != Piece::KING) {
        encodedMove = 0x3;
    } else if (pt) {
        encodedMove = 0x2;
    } else if (pf->getKind() == Piece::PAWN) {
        encodedMove = 0x1;
    }

    /*ppp*/
    uint16_t encodedPiece = 0x0;
    /*fffrrr*/
    uint16_t encodedSquare = 0x0;

    switch (pf->getKind()) {
        case Piece::PAWN:
            encodedPiece = 0x1;
            break;
        case Piece::KNIGHT:
            /*Knight goes to C2*/
            encodedPiece = 0x2;
            encodedSquare = 7 - abs(ft - File::C);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs((int)(rt - 2));
            break;
        case Piece::BISHOP:
            /*Bishop goes to D2*/
            encodedPiece = 0x3;
            encodedSquare = 7 - abs(ft - File::D);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs((int)(rt - 2));
            break;
        case Piece::ROOK:
            /*Rook goes to B3*/
            encodedPiece = 0x4;
            encodedSquare = 7 - abs(ft - File::B);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs((int)(rt - 3));
            break;
        case Piece::QUEEN:
            /*Queen goes to C3*/
            encodedPiece = 0x5;
            encodedSquare = 7 - abs(ft - File::C);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs((int)(rt - 3));
            break;
        case Piece::KING:
            /*King goes to B2*/
            encodedPiece = 0x6;
            encodedSquare = 7 - abs(ft - File::B);
            encodedSquare <<= 3;
            encodedSquare |= 7 - abs((int)(rt - 2));
            break;
    }
    /*mmssssssppp*/
    evaluation = (encodedMove << 9) | (encodedSquare << 3) | encodedPiece;
    return evaluation;
}
