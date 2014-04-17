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
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <map>

#include "polyglot_reference.h"
#include "Output.h"
#include "SimpleChessboard.h"

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
    string line;
    /*Get all the positions from file*/
    while (getline(inputfile, line)) {
        positions.push_back(line);
    }
    inputfile.close();

    Position chessboard;
    chessboard.init();

    map<string, string> failed;

    Out::output("Starting test with " + to_string(positions.size()) + " positions\n");

    int col = 0;
    for (string pos : positions) {
        chessboard.clear();
        try {
            chessboard.set(pos);
            if (DEBUG)
                Out::output(chessboard.pretty());
            uint64_t myHash = chessboard.hash();
            uint64_t expectedHash = pg_ref_hash(chessboard.fen().c_str());

            if (myHash != expectedHash) {
                pair<string, string> fail;
                fprintf(stdout, "%lx vs %lx\n", myHash, expectedHash);
                failed.insert(pair<string, string>(pos, to_string(myHash)
                                             + " vs "
                                             + to_string(expectedHash)));
            }
        } catch (InvalidFenException e) {
            failed.insert(make_pair(pos, "Invalid fen"));
        } catch (...) {
            Out::output("Unexpected exception.\n");
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
            Out::output(test.first);
            Out::output("\n");
            Out::output(test.second);
            Out::output("\n");
        }
    }
}
