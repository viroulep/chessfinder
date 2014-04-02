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
#include "MatfinderOptions.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"
#include "Output.h"

using namespace std;
using namespace Board;


Finder::Finder(int comm) : commId_(comm),
                           pool_(Comm::UCICommunicatorPool::getInstance()),
                           opt_(Options::getInstance())
{
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

void Finder::sendPositionToEngine(Board::Position &pos)
{
    string position = "position fen ";
    position += pos.fen();
    pool_.send(commId_, position);
}


string Finder::getPrettyLines(const Position &pos, const vector<Line> &lines)
{
    string retVal;
    Line curLine;
    int i = 0;
    for (Line cur : lines) {
        if (!cur.empty()) {
            retVal += "\t[" + to_string(++i) + "] ";
            retVal += cur.getPretty(pos.side_to_move() == BLACK);
            retVal += "\n";
        }
        if (i >= opt_.getMaxLines())
            break;
    }
    return retVal;
}

/*FIXME : really useful ?*/
string Finder::getPrettyLine(const Position &pos, const Line &line)
{
    string retVal;
    retVal += line.getPrettyEval(pos.side_to_move() == BLACK);
    retVal += " : ";
    auto moves = line.getMoves();
    int i = 0;
    for (string m : moves) {
        if (i++ < opt_.getMaxLines())
            break;
        if (i > 0)
            retVal += " ";
        retVal += m;
    }
    /*oss << cb_->tryUciMoves(line.getMoves(), limit);*/
    return retVal;
}

/*
 * These are private
 */

