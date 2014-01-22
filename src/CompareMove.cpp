#include "CompareMove.h"



/*Return true if lhs < rhs*/
bool CompareMove::compareTake(Line *lhs, Line *rhs)
{
    return false;
    /*
     *Move lhsM = getMoveFromUci(lhs->firstMove());
     *Move rhsM = getMoveFromUci(rhs->firstMove());
     *if (!lhsM.takePiece && !rhsM.takePiece)
     *    return false;
     *else if (lhsM.takePiece)
     *    return true;
     *else if (rhsM.takePiece)
     *    return false;
     *else {
     *    Piece *lhsP = lhsM.to->getPiece();
     *    Piece *rhsP = rhsM.to->getPiece();
     *    return lhsP->getKind() < rhsP->getKind();
     *}
     */
}

