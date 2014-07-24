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
#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <string>
#include <list>
#include <map>

class MoveComparator;
class Config;

typedef std::list<std::pair<std::string, std::list<std::string>>> PositionList;

enum Variant {
    STANDARD,
    LOS_ALAMOS,
    GARDNER
};

enum SearchMode {
    DEPTH,
    TIME,
    MIXED
};

/*NOTE threshold : certificat pour la table*/
/*TODO rajouter intervalle tri*/
class Options {
    public:

        Variant getVariant() const;
        void setVariant(std::string sv);

        SearchMode getSearchMode() const;
        void setSearchMode(std::string sm);

        int getSearchDepth() const;

        bool buildOracleForWhite() const;
        void setOracleSide(std::string side);

        const std::string &getInputFile() const;
        const std::string &getOutputFile() const;

        const std::string &getTableFolder() const;

        void setInputFile(std::string in);
        void setOutputFile(std::string out);

        void setTableFolder(std::string folder);

        const std::string &getEngineFullpath() const;
        int getEngineHashmapSize() const;
        int getEngineThreads() const;

        int getCutoffThreshold() const;
        int getEngineNumber() const;
        int getPlayforMovetime() const;
        int getPlayagainstMovetime() const;

        int getVerboseLevel() const;
        void setVerboseLevel(int level);

        int getMaxMoves() const;

        int getMateThreshold() const;
        int getMaxLines() const;

        int getBestmoveDeviation() const;

        unsigned int getMaxPiecesEnding() const;
        bool fullBuild() const;

        MoveComparator *getMoveComparator() const;
        void setMoveComparator(MoveComparator *mc);
        void setMoveComparator(std::string smc);

        const PositionList &getPositionList();
        void setPositionList(PositionList &theList);
        void addPositionToList(std::string pos, std::list<std::string> &moves);

        void addConfig(Config &conf);

        static Options &getInstance();
    private:
        Options();
        /*TODO comment*/

        Variant variant_ = STANDARD;

        SearchMode mode_ = TIME;
        int searchDepth_ = 10;

        bool buildOracleForWhite_ = true;

        std::string inputFile_ = "";
        std::string outputFile_ = "";

        std::string engineFullpath_ = "/usr/bin/stockfish";
        int engineHashmapSize_ = 1024;
        int engineThreads_ = 2;

        int finderCutoffThreshold_ = 100;
        int finderEngineNumber = 1;
        int playforMovetime_ = 1500;
        int playagainstMovetime_ = 1000;
        int verboseLevel_ = 0;
        int maxMoves_ = 255;

        int mateThreshold_ = 10000;
        int maxLines_ = 8;

        int bestmoveDeviation_ = 19;

        unsigned int maxPiecesEnding_ = 6;
        /*Build full oracle, or just until we reach an 6 piece ending*/
        bool fullBuild_ = false;

        PositionList positions_;

        MoveComparator *comp_ = nullptr;

        std::string tableFolder_;

        static Options instance_;
};


#endif
