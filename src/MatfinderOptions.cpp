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
#include "Output.h"
#include "MatfinderOptions.h"

using namespace std;

//Init static members
string MatfinderOptions::ENGINE_ = DEFAULT_ENGINE;
string MatfinderOptions::PATH_ = DEFAULT_PATH;
PositionList MatfinderOptions::START_POS_LIST_;
int MatfinderOptions::PLAYFOR_MOVETIME_ = DEFAULT_PLAYFOR_MOVETIME_;
int MatfinderOptions::PLAYAGAINST_MOVETIME_ = DEFAULT_PLAYAGAINST_MOVETIME_;
int MatfinderOptions::MAX_LINES_ = DEFAULT_MAX_LINES;
int MatfinderOptions::VERBOSE_LEVEL_ = DEFAULT_VERBOSE_LEVEL;
int MatfinderOptions::HASHMAP_SIZE_ = DEFAULT_HASHMAP_SIZE;
int MatfinderOptions::CP_TRESHOLD_ = DEFAULT_TRESHOLD;
int MatfinderOptions::MATE_EQUIV_ = DEFAULT_MATE_EQUIV;

int MatfinderOptions::ENGINE_THREADS_ = DEFAULT_THREADS;
string MatfinderOptions::INPUT_ = DEFAULT_INPUT_FILE;
string MatfinderOptions::OUTPUT_ = DEFAULT_OUTPUT_FILE;

/*MoveComparator *MatfinderOptions::MOVE_COMPARATOR_ = new MapMoveComparator;*/

string MatfinderOptions::getEngine() { return ENGINE_; }
void MatfinderOptions::setEngine(string engine) { ENGINE_ = engine; }

string MatfinderOptions::getPath() { return PATH_; }
void MatfinderOptions::setPath(string path) { PATH_ = path; }

const PositionList &MatfinderOptions::getPositionList()
{
    return START_POS_LIST_;
}
void MatfinderOptions::setPositionList(PositionList &theList)
{
    START_POS_LIST_.clear();
    //Delete moves from theList and insert into class member
    START_POS_LIST_.splice(START_POS_LIST_.end(), theList);
}
void MatfinderOptions::addPositionToList(string pos, list<string> &moves)
{
    pair<string, list<string>> toAdd(pos, moves);
    START_POS_LIST_.push_front(toAdd);
}

int MatfinderOptions::getPlayagainstMovetime()
{
    return PLAYAGAINST_MOVETIME_;
}
void MatfinderOptions::setPlayagainstMovetime(int movetime)
{
    PLAYAGAINST_MOVETIME_ = movetime;
}

int MatfinderOptions::getPlayforMovetime() { return PLAYFOR_MOVETIME_; }
void MatfinderOptions::setPlayforMovetime(int movetime)
{
    PLAYFOR_MOVETIME_ = movetime;
}

int MatfinderOptions::getVerboseLevel() { return VERBOSE_LEVEL_; }
void MatfinderOptions::setVerboseLevel(int verboseLevel)
{
    VERBOSE_LEVEL_ = verboseLevel;
}

int MatfinderOptions::getMaxLines() { return MAX_LINES_; }
void MatfinderOptions::setMaxLines(int maxLines) { MAX_LINES_ = maxLines; }

int MatfinderOptions::getHashmapSize() { return HASHMAP_SIZE_; }
void MatfinderOptions::setHashmapSize(int size)
{
    HASHMAP_SIZE_ = size;
}

int MatfinderOptions::getCpTreshold() { return CP_TRESHOLD_; }

void MatfinderOptions::setCpTreshold(int treshold)
{
    CP_TRESHOLD_ = treshold;
}

int MatfinderOptions::getEngineThreads() { return ENGINE_THREADS_; }
void MatfinderOptions::setEngineThreads(int threads) { ENGINE_THREADS_ = threads; }


int MatfinderOptions::getMateEquiv() { return MATE_EQUIV_; }

void MatfinderOptions::setMateEquiv(int mate_equiv) { MATE_EQUIV_ = mate_equiv; }

string MatfinderOptions::getOutputFile()
{
    return OUTPUT_;
}
void MatfinderOptions::setOutputFile(string file)
{
    OUTPUT_ = file;
}

string MatfinderOptions::getInputFile()
{
    return INPUT_;
}
void MatfinderOptions::setInputFile(string file)
{
    INPUT_ = file;
}

/*
 *MoveComparator *MatfinderOptions::getMoveComparator()
 *{
 *    if (!MOVE_COMPARATOR_)
 *        Err::handle("MoveComparator used but not initialized");
 *    return MOVE_COMPARATOR_;
 *}
 *
 *void MatfinderOptions::setMoveComparator(MoveComparator *mc)
 *{
 *    if (MOVE_COMPARATOR_)
 *        delete MOVE_COMPARATOR_;
 *    MOVE_COMPARATOR_ = mc;
 *}
 *
 *void MatfinderOptions::setMoveComparator(string smc)
 *{
 *    MoveComparator *mc = nullptr;
 *    if (smc == "map") {
 *        mc = new MapMoveComparator;
 *    } else if (smc == "default") {
 *        mc = new DefaultMoveComparator;
 *     [>Sample adding of a comparator :<]
 *    [>} else if (smc == "sample") {<]
 *        [>mc = new SampleMoveComparator;<]
 *    } else {
 *        Err::handle("Unknown move comparator : " + smc);
 *    }
 *    setMoveComparator(mc);
 *}
 *
 *
 */
string MatfinderOptions::getPretty()
{
    ostringstream oss;
    oss << "MatfinderOptions :\n";
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
