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

//Class representing a particular line for a position
class Line {
public:
    Line();
    Line(float ev, int depth, std::list<std::string> mv, bool isMat = false);
    //Return more readable format
    std::string getPretty(bool invertEval) const;
    std::string getPrettyEval(bool invertEval) const;

    void update(Line &line);
    bool isMat() const;
    float getEval() const;
    bool empty() const;
    std::string firstMove() const;
    const std::list<std::string> &getMoves() const;
    bool operator<(const Line &rhs);

    static bool compareLineLength(Line *lhs, Line *rhs);
private:
    //Line Evaluation
    //Might be a mat
    int eval_ = 0;
    bool isMat_ = false;
    int depth_ = 0;
    std::list<std::string> moves_;
};

#endif
