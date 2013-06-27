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
#include <cmath>
#include <iostream>
#include <sstream>
#include "Line.h"
#include "MatFinderOptions.h"

Line::Line() : eval_(0)
{
    moves_.clear();
}

Line::Line(float ev, int depth, list<string> mv, bool isMat) :
    eval_(ev), depth_(depth), isMat_(isMat)
{
    //*copy* the moves from mv to move_
    moves_.insert(moves_.end(), mv.begin(), mv.end());
}

string Line::getPrettyEval(bool invertEval)
{

    ostringstream out;

    float displayEval = (invertEval)?-eval_:eval_;
    if (isMat_)
        out << "#";
    else
        displayEval /= 100;
    char eval[10];
    //Tricky: avoid invalid write of '0' in output stream
    sprintf(eval, "%.2f", displayEval);
    out << eval;
    return out.str();
}

//TODO: getPretty(bool invertEval) ?! (so that Line doesn't depend on board)
string Line::getPretty(bool invertEval)
{
    ostringstream out;

    float displayEval = (invertEval)?-eval_:eval_;
    if (isMat_)
        out << "#";
    else
        displayEval /= 100;
    char eval[10];
    //Tricky: avoid invalid write of '0' in output stream
    sprintf(eval, "%.2f", displayEval);
    out << eval;
    out << ", d: ";
    out << depth_;
    out << ", line : ";
    list<string> tmpList = moves_;
    int i = 0;
    while (!tmpList.empty() && i < MatFinderOptions::movesDisplayed) {
        out << tmpList.front();
        out << " ";
        tmpList.pop_front();
        i++;
    }
    if (i < tmpList.size())
        out << "[...moves...]";
    out << "\n";

    return out.str();
}

const list<string> &Line::getMoves()
{
    return moves_;
}

void Line::update(Line &line)
{
    eval_ = line.eval_;
    depth_ = line.depth_;
    moves_.clear();
    //clear and copy
    moves_.insert(moves_.end(), line.moves_.begin(), line.moves_.end());
    isMat_ = line.isMat_;
    //update(line.eval_, line.depth_, line.moves_, line.isMat_);
}

bool Line::isMat()
{
    return isMat_;
}

float Line::getEval()
{
    return eval_;
}

bool Line::empty()
{
    return moves_.empty();
}

string Line::firstMove()
{
    return moves_.front();
}

bool Line::operator<(const Line &rhs)
{
    return fabs(eval_) < fabs(rhs.eval_);
}

Line Line::emptyLine;
