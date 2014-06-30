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
#include <algorithm>
#include "Options.h"
#include "ConfigParser.h"
#include "Output.h"
#include "CompareMove.h"

using namespace std;

Options Options::instance_ = Options();

Variant Options::getVariant() const
{
    return variant_;
}

void Options::setVariant(string sv)
{
    if (sv == "standard")
        variant_ = STANDARD;
    else if (sv == "gardner")
        variant_ = GARDNER;
    else if (sv == "alamos" || sv == "losalamos")
        variant_ = LOS_ALAMOS;
    else
        Err::handle("Unrecognize chess variant : " + sv);
}

SearchMode Options::getSearchMode() const
{
    return mode_;
}

void Options::setSearchMode(string sm)
{
    if (sm == "depth")
        mode_ = DEPTH;
    else if (sm == "time")
        mode_ = TIME;
    else if (sm == "mixed")
        mode_ = MIXED;
    else
        Err::handle("Unrecognize search mode : " + sm);
}

int Options::getSearchDepth() const
{
    return searchDepth_;
}

bool Options::buildOracleForWhite() const
{
    return buildOracleForWhite_;
}

void Options::setOracleSide(string side)
{
    if (side == "white")
        buildOracleForWhite_ = true;
    else if (side == "black")
        buildOracleForWhite_ = false;
    else
        Err::handle("Unrecognize side : " + side);
}

const string &Options::getInputFile() const
{
    return inputFile_;
}

const string &Options::getOutputFile() const
{
    return outputFile_;
}

const map<string, string> &Options::getInputTables() const
{
    return inputTables_;
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

int Options::getEngineNumber() const
{
    return finderEngineNumber;
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

void Options::setVerboseLevel(int level)
{
    verboseLevel_ = level;
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

MoveComparator *Options::getMoveComparator() const
{
    if (!comp_)
        Err::handle("MoveComparator used but not initialized");
    return comp_;
}

void Options::setMoveComparator(MoveComparator *mc)
{
    if (comp_)
        delete comp_;
    comp_ = mc;
}

void Options::setMoveComparator(string smc)
{
    MoveComparator *mc = nullptr;
    if (smc == "map") {
        mc = new MapMoveComparator;
    } else if (smc == "default") {
        mc = new DefaultMoveComparator;
     /*Sample adding of a comparator :*/
    /*} else if (smc == "sample") {*/
        /*mc = new SampleMoveComparator;*/
    } else {
        Err::handle("Unknown move comparator : " + smc);
    }
    setMoveComparator(mc);
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

    val = conf("engine", "variant");
    if (val)
        setVariant(val);

    /*Getting Finder configuration*/
    val = conf("finder", "engine_number");
    PARSE_INTVAL(finderEngineNumber, "engine_number");

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

    /*Getting OracleFinder specific configuration*/
    val = conf("oraclefinder", "comparator");
    if (val)
        setMoveComparator(val);
    val = conf("oraclefinder", "oracle_side");
    if (val)
        setOracleSide(val);
    val = conf("oraclefinder", "search_mode");
    if (val)
        setSearchMode(val);

    /*Getting all input tables*/
    int index = 1;
    Value *key;
    string tableSection = "oracletables";
    while ((key = conf.ikey(tableSection.c_str(), index))
           && (val = conf.ivalue(tableSection.c_str(), index))) {
        string skey = key->get();
        std::sort(skey.begin(), skey.end());
        inputTables_[skey] = val;
        index++;
    }

    val = conf("oraclefinder", "search_depth");
    PARSE_INTVAL(searchDepth_, "search_depth");

}

#undef PARSE_INTVAL

Options::Options()
{
}

Options &Options::getInstance()
{
    return instance_;
}
