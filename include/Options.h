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
#ifndef __MATFINDEROPTIONS_H__
#define __MATFINDEROPTIONS_H__

#include <string>
#include <list>
#include <utility>

#include "CompareMove.h"

#define DEFAULT_MAX_LINES 8
#define DEFAULT_VERBOSE_LEVEL 0
#define DEFAULT_HASHMAP_SIZE 4096
#define DEFAULT_PLAYFOR_MOVETIME_ 2500
#define DEFAULT_PLAYAGAINST_MOVETIME_ 2000
#define DEFAULT_OUTPUT_FILE ""
#define DEFAULT_INPUT_FILE ""
#define DEFAULT_STARTPOS "startpos"
#define DEFAULT_ENGINE "stockfish"
#define DEFAULT_PATH "/usr/local/bin"
#define DEFAULT_TRESHOLD 100
#define DEFAULT_MATE_EQUIV 100000
#define DEFAULT_THREADS 4

#define TIMEOUT_READY 5
#define MOVES_DISPLAYED 5

typedef std::list<std::pair<std::string, std::list<std::string>>> PositionList;

class Options {
public:

    static std::string getEngine();
    static void setEngine(std::string engine);

    static std::string getPath();
    static void setPath(std::string path);

    static const PositionList &getPositionList();
    static void setPositionList(PositionList &theList);
    static void addPositionToList(std::string pos, std::list<std::string> &moves);

    static int getPlayagainstMovetime();
    static void setPlayagainstMovetime(int movetime);

    static int getPlayforMovetime();
    static void setPlayforMovetime(int movetime);

    static int getVerboseLevel();
    static void setVerboseLevel(int verboseLevel);

    static int getMaxLines();
    static void setMaxLines(int maxLines);

    static int getHashmapSize();
    static void setHashmapSize(int size);

    static int getCpTreshold();
    static void setCpTreshold(int treshold);

    static int getMateEquiv();
    static void setMateEquiv(int treshold);

    static int getEngineThreads();
    static void setEngineThreads(int threads);

    static std::string getOutputFile();
    static void setOutputFile(std::string file);

    static std::string getInputFile();
    static void setInputFile(std::string file);

    static MoveComparator *getMoveComparator();
    static void setMoveComparator(MoveComparator *mc);
    static void setMoveComparator(std::string smc);


    //Pretty-print the options
    static std::string getPretty();

    //How long we wait for the engine to be ready (s)
    const static int isreadyTimeout = TIMEOUT_READY;

    //Number of moves displayed when printing lines
    const static int movesDisplayed = MOVES_DISPLAYED;

private:
    //Private (static class)
    Options();

    static std::string ENGINE_;
    static std::string PATH_;


    // in ms
    static int PLAYFOR_MOVETIME_;
    static int PLAYAGAINST_MOVETIME_;

    static int MAX_LINES_;

    //in centipawn
    static int CP_TRESHOLD_;

    /*
     * Value equivalent to mate (if we want to close the position at a given
     * evaluation
     */
    static int MATE_EQUIV_;

    // 0 = minimal output
    // 1 = not-that-huge output
    // [...]
    // 5 = display too many things
    static int VERBOSE_LEVEL_;

    // in Mo
    static int HASHMAP_SIZE_;

    /*
     * Stores all the position to be processed :
     * a list of pair (starting position, user moves)
     */
    static PositionList START_POS_LIST_;

    //Number of threads used by stockfish
    static int ENGINE_THREADS_;

    //The file loaded or exported
    static std::string INPUT_;
    static std::string OUTPUT_;

    // The move comparator to be used
    static MoveComparator *MOVE_COMPARATOR_;
};


#endif
