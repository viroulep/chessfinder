/*
 * Oraclefinder, a program to find "perfect" chess game
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
#ifndef __ORACLEFINDER_H__
#define __ORACLEFINDER_H__

#include <string>
#include <vector>
#include <list>
#include <vector>
#include <unordered_map>
#include "Engine.h"
#include "Thread.h"
#include "Stream.h"
#include "Line.h"
#include "Finder.h"
#include "Hashing.h"

using namespace std;

//Forward decl
class UCIReceiver;





typedef array<vector<Line *>, 2> SortedLines;

class OracleFinder : public Finder {
public:
    OracleFinder();
    virtual ~OracleFinder();

private:
    int runFinderOnCurrentPosition();
    SortedLines getLines();
    void proceedUnbalancedLines(vector<Line *> unbalanced);
    HashTable *oracleTable_;
    /*
     *Node *rootNode_ = NULL;
     */
    list<Node *> toProceed_;
};

#endif
