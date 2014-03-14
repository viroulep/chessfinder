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
#include <fstream>
#include <algorithm>
#include <cstdlib>

#include "Movegen.h"
#include "Output.h"
#include "SimpleChessboard.h"
#include "UCICommunicator.h"

using namespace std;
using namespace Board;

#define DEBUG 0

int diff(vector<string> &lhs, vector<string> &rhs) {
    int diff = 0;
    sort(lhs.begin(), lhs.end());
    sort(rhs.begin(), rhs.end());
    if (DEBUG) {
        for (string s : lhs)
            Out::output(s + ", ");
        Out::output("\n---\n");
        for (string s : rhs)
            Out::output(s + ", ");
        Out::output("\n---\n");
    }
    auto lhsIt = lhs.begin();
    auto rhsIt = rhs.begin();

    while (lhsIt != lhs.end() && rhsIt != rhs.end()) {
        if (*lhsIt != *rhsIt)
            diff++;
        ++lhsIt;
        ++rhsIt;
    }

    if (lhsIt != lhs.end() || rhsIt != rhs.end()) {
        if (rhs.size() >= lhs.size())
            diff += rhs.size() - lhs.size();
        else
            diff += lhs.size() - rhs.size();
    }
    return diff;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        Err::handle("You must provide an input file with fen positions");

    char *filename_cstr = argv[1];
    string filename(filename_cstr);
    Out::output("Using input file : " + filename + "\n");
    ifstream inputfile(filename, ifstream::in);
    if (!inputfile.good())
        Err::handle("Error : unable to open file.");

    vector<string> positions;
    /*positions.push_back("1b1r2k1/2qr2pp/p3p3/Pp1bPpB1/1P4R1/2BB3P/2P3P1/5R1K w - f6 0 29");*/
    /*positions.push_back("1b1r2k1/1p3ppp/8/1NpP4/r7/4P3/2pN1PPP/R3K2R w KQ - 0 19");*/
    string line;
    /*Get all the positions from file*/
    while (getline(inputfile, line)) {
        positions.push_back(line);
    }
    inputfile.close();

    /*Setup some engine options*/
    Comm::EngineOptions engine_options;
    engine_options.insert(make_pair("MultiPV", "254"));
    engine_options.insert(make_pair("UCI_AnalyseMode", "true"));

    Comm::UCICommunicatorPool &pool = Comm::UCICommunicatorPool::getInstance();

    /*Create the engine and its communicator*/
    /*FIXME get engine path from config*/
    int commId = pool.create<Comm::LocalUCICommunicator>("/usr/local/bin/stockfish",
                                                         engine_options);
    Position chessboard;
    chessboard.init();
    vector<string> boardMoves;
    vector<string> engineMoves;

    pool.send(commId, "ucinewgame");
    if (!pool.isReady(commId))
        exit(EXIT_FAILURE);

    map<string, int> failed;

    Out::output("Starting test with " + to_string(positions.size()) + " positions\n");

    int col = 0;
    for (string pos : positions) {
        chessboard.clear();
        boardMoves.clear();
        engineMoves.clear();
        try {
            chessboard.set(pos);
            if (DEBUG)
                Out::output(chessboard.pretty());
            /*
             * This has way too many loops, but it works and I'm too
             * lazy to optimize it.
             */

            /*Generate board moves*/
            for (Move m : Board::gen_all(chessboard)) {
                boardMoves.push_back(Board::move_to_string(m));
            }

            pool.send(commId, "position fen " + pos);
            if (!pool.isReady(commId))
                exit(EXIT_FAILURE);

            /*Generate engine moves*/
            pool.sendAndWaitBestmove(commId, "go depth 1");
            for (Line l : pool.getResultLines(commId)) {
                if (l.empty())
                    break;
                engineMoves.push_back(l.firstMove());
            }

            int movediff = diff(boardMoves, engineMoves);
            if (movediff)
                failed.insert(make_pair(pos, movediff));
        } catch (InvalidFenException e) {
            failed.insert(make_pair(pos, -1));
        }
        col++;
        if (col % 10000 == 0) {
            Out::output("\n10000 positions tested (" + to_string(failed.size())
                        + " failed)\n");
            if (failed.size() > 0)
                break;
        }
    }
    Out::output("\n");

    Out::output("End of tests\n");
    Out::output("Test passed : " + to_string(positions.size() - failed.size())
                + "/" + to_string(positions.size()) + "\n");
    if (failed.size() > 0) {
        Out::output("Tests failed :\n");
        for (auto test : failed) {
            Out::output("  -\"" + test.first + "\" ");
            if (test.second > 0)
                Out::output(" has " + to_string(test.second)
                            + " differences with engine.");
            else
                Out::output("is not a valid position.");
            Out::output("\n");
        }
    }
}
