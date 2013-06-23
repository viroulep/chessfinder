
#include <iostream>
#include "Board.h"
#include "Utils.h"

using namespace std;
using namespace Board;

int main()
{
    Chessboard test;
    Side tSide = Side::WHITE;
    File tFile = G;
    Rank tRank = 6;
    Utils::output("Side : " + to_string(tSide) + "\n");
    Utils::output(string("File : ") + to_char(tFile) + "\n");
    Utils::output(string("Rank : ") + to_char(tRank) + "\n");
    return 0;
}
