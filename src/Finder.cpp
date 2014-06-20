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
#include <sstream>
#include <cmath>
#include <unistd.h>

#include "Finder.h"
#include "Stream.h"
#include "Utils.h"
#include "Output.h"

using namespace std;
using namespace Board;

Options &Finder::opt_ = Options::getInstance();
Comm::UCICommunicatorPool &Finder::pool_ = Comm::UCICommunicatorPool::getInstance();
Board::Color Finder::playFor_;
std::vector<int> Finder::commIds_;


Finder::Finder(vector<int> &commIds)
{
    commIds_.insert(commIds_.end(), commIds.begin(), commIds.end());
}

Finder::~Finder()
{
}

int Finder::runFinder()
{
    /*pos.set("rnb1kb1r/ppp1pppp/5n2/3q4/8/2N5/PPPP1PPP/R1BQKBNR w KQkq - 2 4");*/

    const PositionList &allPositions = opt_.getPositionList();

    for (auto startinpos : allPositions) {
        Position pos;
        /*Extract infos from pair*/
        pos.set(startinpos.first);
        list<string> userMoves = startinpos.second;

        Out::output("Running finder on \"" + pos.fen() + "\", with moves : "
                    + Utils::listToString(userMoves) + "\n");


        /*Fresh finder*/
        addedMoves_ = 0;
        /*Run*/
        runFinderOnPosition(pos, userMoves);

    }

    if (allPositions.empty())
        Out::output("No position to run the finder on. Please adjust"\
                    " --startingpos and/or --position_file\n");

    return EXIT_SUCCESS;
}

void Finder::sendPositionToEngine(Board::Position &pos, int commId)
{
    string position = "position fen ";
    position += pos.fen();
    pool_.send(commId, position);
}


/*
 * These are private
 */

