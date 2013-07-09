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
#include "OracleFinder.h"
#include "Options.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"



OracleFinder::OracleFinder() : Finder()
{
    //engine_side_ = cb_->getActiveSide();
    engine_play_for_ = cb_->getActiveSide();
    //engine_play_for_ = Options::getPlayFor();
}

OracleFinder::~OracleFinder()
{
}


int OracleFinder::runFinderOnCurrentPosition()
{
    Board::Side sideToMove = cb_->getActiveSide();

    Utils::output("Starting board is :\n" + cb_->to_string() + "\n");
    Utils::output("Doing some basic evaluation on submitted position...\n");

    sendCurrentPositionToEngine();
    sendToEngine("go movetime "
            + to_string(Options::getPlayforMovetime()));
    waitBestmove();
    Utils::output("Evaluation is :\n");
    Utils::output(getPrettyLines());

    //Main loop
    while (true) {
        Line bestLine;
        Side active = cb_->getActiveSide();

        Utils::output("[" + Board::to_string(active)
                + "] Depth " + to_string(addedMoves_) + "\n");

        sendCurrentPositionToEngine();

        Utils::output(cb_->to_string(), 2);

        //Thinking according to the side the engine play for
        int moveTime = Options::getPlayforMovetime();


        //Initialize vector with empty lines
        //TODO change
        lines_.assign(Options::getMaxLines(), Line::emptyLine);

        sendToEngine("go movetime " + to_string(moveTime));

        Utils::output("[" + Board::to_string(active)
                + "] Thinking... (" + to_string(moveTime) + ")\n", 1);

        //Wait for engine to finish thinking
        waitBestmove();

        Utils::output(getPrettyLines(), 2);
    }

    //Display info at the end of computation
    Utils::output("[End] Finder is done. Starting board was : \n");
    Utils::output(cb_->to_string() + "\n");


    return 0;
}


