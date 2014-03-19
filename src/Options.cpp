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
#include "Options.h"
#include "Output.h"

using namespace std;

Options Options::instance_ = Options();

Variant Options::getVariant() const
{
    return variant_;
}

const string &Options::getInputFile() const
{
    return inputFile_;
}

const string &Options::getOutputFile() const
{
    return outputFile_;
}

void Options::setInputFile(string in)
{
    inputFile_ = in;
}

void Options::setOutputFile(string out)
{
    outputFile_ = out;
}

const string &Options::getEngineFullpath() const
{
    return engineFullpath_;
}

int Options::getEngineHashmapSize() const
{
    return engineHashmapSize_;
}

int Options::getEngineThreads() const
{
    return engineThreads_;
}

int Options::getCutoffTreshold() const
{
    return finderCutoffTreshold_;
}

int Options::getPlayforMovetime() const
{
    return playforMovetime_;
}

int Options::getPlayagainstMovetime() const
{
    return playagainstMovetime_;
}

int Options::getVerboseLevel() const
{
    return verboseLevel_;
}

int Options::getMaxMoves() const
{
    return maxMoves_;
}

int Options::getMateTreshold() const
{
    return mateTreshold_;
}

int Options::getMaxLines() const
{
    return maxLines_;
}

const PositionList &Options::getPositionList()
{
    return positions_;
}

void Options::setPositionList(PositionList &theList)
{
    positions_.clear();
    //Delete moves from theList and insert into class member
    positions_.splice(positions_.end(), theList);
}

void Options::addPositionToList(string pos, list<string> &moves)
{
    positions_.push_front(make_pair(pos, moves));
}

#define PARSE_INTVAL(option, optionName) \
if (val) {\
    try {\
        intVal = stoi(val);\
        option = intVal;\
    } catch (...) {\
        Err::handle("Error parsing " optionName " from configuration file");\
    }\
}

void Options::addConfig(Config &conf)
{
    Out::output("Setting options from configuration file.\n", 1);
    const char *val;
    int intVal;
    /*Getting engine configuration*/
    val = conf("engine", "path");
    if (val)
        engineFullpath_ = val;

    val = conf("engine", "hashmap_size");
    PARSE_INTVAL(engineHashmapSize_, "hashmap_size");

    val = conf("engine", "threads");
    PARSE_INTVAL(engineThreads_, "threads number");

    /*Getting Finder configuration*/
    val = conf("finder", "cutoff_treshold");
    PARSE_INTVAL(finderCutoffTreshold_, "cutoff_treshold");

    val = conf("finder", "playfor_movetime");
    PARSE_INTVAL(playforMovetime_, "playfor_movetime");

    val = conf("finder", "playagainst_movetime");
    PARSE_INTVAL(playagainstMovetime_, "playagainst_movetime");

    val = conf("finder", "verbose_level");
    PARSE_INTVAL(verboseLevel_, "verbose_level");

    /*Getting MatFinder specific configuration*/
    val = conf("matfinder", "lines");
    PARSE_INTVAL(maxLines_, "lines");

    val = conf("matfinder", "mate_treshold");
    PARSE_INTVAL(mateTreshold_, "mate_treshold");

}

#undef PARSE_INTVAL

Options::Options()
{
}

Options &Options::getInstance()
{
    return instance_;
}
