
#include <iostream>
#include "Chessboard.h"
#include "Utils.h"
#include "MatFinderOptions.h"

using namespace std;
using namespace Board;

int main()
{
    MatFinderOptions::setVerboseLevel(1);
    Chessboard *test = Chessboard::createChessboard();
    Utils::output(string("Chessboard : \n") + test->to_string());
    delete test;
    return 0;
}
