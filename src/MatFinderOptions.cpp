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
#include <sstream>
#include "Utils.h"
#include "MatFinderOptions.h"



//Init static members
string MatFinderOptions::ENGINE_ = DEFAULT_ENGINE;
string MatFinderOptions::PATH_ = DEFAULT_PATH;
PositionList MatFinderOptions::START_POS_LIST_;
int MatFinderOptions::PLAYFOR_MOVETIME_ = DEFAULT_PLAYFOR_MOVETIME_;
int MatFinderOptions::PLAYAGAINST_MOVETIME_ = DEFAULT_PLAYAGAINST_MOVETIME_;
int MatFinderOptions::MAX_LINES_ = DEFAULT_MAX_LINES;
Board::Side MatFinderOptions::PLAY_FOR_ = Board::Side::DEFAULT_PLAY_FOR;
int MatFinderOptions::VERBOSE_LEVEL_ = DEFAULT_VERBOSE_LEVEL;
int MatFinderOptions::HASHMAP_SIZE_ = DEFAULT_HASHMAP_SIZE;
int MatFinderOptions::CP_TRESHOLD_ = DEFAULT_TRESHOLD;



string MatFinderOptions::getEngine() { return ENGINE_; }
void MatFinderOptions::setEngine(string engine) { ENGINE_ = engine; }

string MatFinderOptions::getPath() { return PATH_; }
void MatFinderOptions::setPath(string path) { PATH_ = path; }

const PositionList &MatFinderOptions::getPositionList()
{
    return START_POS_LIST_;
}
void MatFinderOptions::setPositionList(PositionList &theList)
{
    START_POS_LIST_.clear();
    //Delete moves from theList and insert into class member
    START_POS_LIST_.splice(START_POS_LIST_.end(), theList);
}
void MatFinderOptions::addPositionToList(string pos, list<string> &moves)
{
    pair<string, list<string>> toAdd(pos, moves);
    START_POS_LIST_.push_front(toAdd);
}

int MatFinderOptions::getPlayagainstMovetime()
{
    return PLAYAGAINST_MOVETIME_;
}
void MatFinderOptions::setPlayagainstMovetime(int movetime)
{
    PLAYAGAINST_MOVETIME_ = movetime;
}

int MatFinderOptions::getPlayforMovetime() { return PLAYFOR_MOVETIME_; }
void MatFinderOptions::setPlayforMovetime(int movetime)
{
    PLAYFOR_MOVETIME_ = movetime;
}

int MatFinderOptions::getVerboseLevel() { return VERBOSE_LEVEL_; }
void MatFinderOptions::setVerboseLevel(int verboseLevel)
{
    VERBOSE_LEVEL_ = verboseLevel;
}

Board::Side MatFinderOptions::getPlayFor() { return PLAY_FOR_; }
void MatFinderOptions::setPlayFor(Board::Side playFor) { PLAY_FOR_ = playFor; }

int MatFinderOptions::getMaxLines() { return MAX_LINES_; }
void MatFinderOptions::setMaxLines(int maxLines) { MAX_LINES_ = maxLines; }

int MatFinderOptions::getHashmapSize() { return HASHMAP_SIZE_; }
void MatFinderOptions::setHashmapSize(int size)
{
    HASHMAP_SIZE_ = size;
}

int MatFinderOptions::getCpTreshold() { return CP_TRESHOLD_; }

void MatFinderOptions::setCpTreshold(int treshold)
{
    CP_TRESHOLD_ = treshold;
}


string MatFinderOptions::getPretty()
{
    ostringstream oss;
    oss << "Options :\n";
    oss << "\t" << "Engine\t\t\t" << " = " << ENGINE_ << endl;
    oss << "\t" << "Path\t\t\t" << " = " << PATH_ << endl;
    oss << "\t" << "Playfor\t\t\t" << " = " << Board::to_string(PLAY_FOR_) << endl;
    oss << "\t" << "Playfor Movetime\t" << " = " << PLAYFOR_MOVETIME_ << endl;
    oss << "\t" << "Playagainst Movetime\t" << " = "
        << PLAYAGAINST_MOVETIME_ << endl;
    oss << "\t" << "Max lines\t\t" << " = " << MAX_LINES_ << endl;
    oss << "\t" << "Verbose level\t\t" << " = " << VERBOSE_LEVEL_ << endl;
    oss << "\t" << "Hashmap size\t\t" << " = " << HASHMAP_SIZE_ << endl;
    oss << "\t" << "Centipawn treshold\t" << " = " << CP_TRESHOLD_ << endl;

    return oss.str();
}
