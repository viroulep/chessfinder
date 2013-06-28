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
#ifndef __LINE_H__
#define __LINE_H__

#include <string>
#include <list>
#include "Board.h"

using namespace std;

//Class representing a particular line for a position
class Line {
public:
    Line();
    Line(float ev, int depth, list<string> mv, bool isMat = false);
    //Return more readable format
    string getPretty(bool invertEval);
    string getPrettyEval(bool invertEval);

    void update(Line &line);
    bool isMat();
    float getEval();
    bool empty();
    string firstMove();
    const list<string> &getMoves();
    bool operator<(const Line &rhs);

    static Line emptyLine;
private:
    //Line Evaluation
    //Might be a mat
    int eval_;
    bool isMat_ = false;
    int depth_;
    list<string> moves_;
};

#endif
