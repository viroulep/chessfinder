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
#include "Options.h"



//Init static members
string Options::ENGINE_ = DEFAULT_ENGINE;
string Options::PATH_ = DEFAULT_PATH;
PositionList Options::START_POS_LIST_;
int Options::PLAYFOR_MOVETIME_ = DEFAULT_PLAYFOR_MOVETIME_;
int Options::PLAYAGAINST_MOVETIME_ = DEFAULT_PLAYAGAINST_MOVETIME_;
int Options::MAX_LINES_ = DEFAULT_MAX_LINES;
int Options::VERBOSE_LEVEL_ = DEFAULT_VERBOSE_LEVEL;
int Options::HASHMAP_SIZE_ = DEFAULT_HASHMAP_SIZE;
int Options::CP_TRESHOLD_ = DEFAULT_TRESHOLD;
int Options::MATE_EQUIV_ = DEFAULT_MATE_EQUIV;

int Options::ENGINE_THREADS_ = DEFAULT_THREADS;
string Options::INPUT_ = DEFAULT_INPUT_FILE;
string Options::OUTPUT_ = DEFAULT_OUTPUT_FILE;

string Options::getEngine() { return ENGINE_; }
void Options::setEngine(string engine) { ENGINE_ = engine; }

string Options::getPath() { return PATH_; }
void Options::setPath(string path) { PATH_ = path; }

const PositionList &Options::getPositionList()
{
    return START_POS_LIST_;
}
void Options::setPositionList(PositionList &theList)
{
    START_POS_LIST_.clear();
    //Delete moves from theList and insert into class member
    START_POS_LIST_.splice(START_POS_LIST_.end(), theList);
}
void Options::addPositionToList(string pos, list<string> &moves)
{
    pair<string, list<string>> toAdd(pos, moves);
    START_POS_LIST_.push_front(toAdd);
}

int Options::getPlayagainstMovetime()
{
    return PLAYAGAINST_MOVETIME_;
}
void Options::setPlayagainstMovetime(int movetime)
{
    PLAYAGAINST_MOVETIME_ = movetime;
}

int Options::getPlayforMovetime() { return PLAYFOR_MOVETIME_; }
void Options::setPlayforMovetime(int movetime)
{
    PLAYFOR_MOVETIME_ = movetime;
}

int Options::getVerboseLevel() { return VERBOSE_LEVEL_; }
void Options::setVerboseLevel(int verboseLevel)
{
    VERBOSE_LEVEL_ = verboseLevel;
}

int Options::getMaxLines() { return MAX_LINES_; }
void Options::setMaxLines(int maxLines) { MAX_LINES_ = maxLines; }

int Options::getHashmapSize() { return HASHMAP_SIZE_; }
void Options::setHashmapSize(int size)
{
    HASHMAP_SIZE_ = size;
}

int Options::getCpTreshold() { return CP_TRESHOLD_; }

void Options::setCpTreshold(int treshold)
{
    CP_TRESHOLD_ = treshold;
}

int Options::getEngineThreads() { return ENGINE_THREADS_; }
void Options::setEngineThreads(int threads) { ENGINE_THREADS_ = threads; }


int Options::getMateEquiv() { return MATE_EQUIV_; }

void Options::setMateEquiv(int mate_equiv) { MATE_EQUIV_ = mate_equiv; }

string Options::getOutputFile()
{
    return OUTPUT_;
}
void Options::setOutputFile(string file)
{
    OUTPUT_ = file;
}

string Options::getInputFile()
{
    return INPUT_;
}
void Options::setInputFile(string file)
{
    INPUT_ = file;
}


string Options::getPretty()
{
    ostringstream oss;
    oss << "Options :\n";
    oss << "\t" << "Engine\t\t\t" << " = " << ENGINE_ << endl;
    oss << "\t" << "Path\t\t\t" << " = " << PATH_ << endl;
    oss << "\t" << "Playfor Movetime\t" << " = " << PLAYFOR_MOVETIME_ << endl;
    oss << "\t" << "Playagainst Movetime\t" << " = "
        << PLAYAGAINST_MOVETIME_ << endl;
    oss << "\t" << "Max lines\t\t" << " = " << MAX_LINES_ << endl;
    oss << "\t" << "Verbose level\t\t" << " = " << VERBOSE_LEVEL_ << endl;
    oss << "\t" << "Hashmap size\t\t" << " = " << HASHMAP_SIZE_ << endl;
    oss << "\t" << "Centipawn treshold\t" << " = " << CP_TRESHOLD_ << endl;
    oss << "\t" << "Eval equivalent to mate  \t" << " = "
        << MATE_EQUIV_ << endl;
    oss << "\t" << "Engine threads  \t" << " = "
        << ENGINE_THREADS_ << endl;
    return oss.str();
}
