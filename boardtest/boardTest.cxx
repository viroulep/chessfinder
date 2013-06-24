
#include <iostream>
#include "Board.h"
#include "Utils.h"
#include "MatFinderOptions.h"

using namespace std;
using namespace Board;

int main()
{
    //MatFinderOptions::setVerboseLevel(5);
    Chessboard *test = Chessboard::createChessboard();
    Side tSide = Side::WHITE;
    File tFile = G;
    Rank tRank = 6;
    /*
     *Utils::output("Side : " + to_string(tSide) + "\n");
     *Utils::output(string("File : ") + to_char(tFile) + "\n");
     *Utils::output(string("Rank : ") + to_char(tRank) + "\n");
     */
    Utils::output(string("Chessboard : \n") + test->to_string());
    delete test;
    return 0;
}
