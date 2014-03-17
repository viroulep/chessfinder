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

#include "ConfigParser.h"



typedef std::list<std::pair<std::string, std::list<std::string>>> PositionList;

class Options {
    public:

        const std::string &getInputFile() const;
        const std::string &getOutputFile() const;

        void setInputFile(std::string in);
        void setOutputFile(std::string out);

        const std::string &getEngineFullpath() const;
        int getEngineHashmapSize() const;
        int getEngineThreads() const;

        int getCutoffTreshold() const;
        int getPlayforMovetime() const;
        int getPlayagainstMovetime() const;
        int getVerboseLevel() const;
        int getMaxMoves() const;

        int getMateTreshold() const;
        int getMaxLines() const;

        const PositionList &getPositionList();
        void setPositionList(PositionList &theList);
        void addPositionToList(std::string pos, std::list<std::string> &moves);

        void addConfig(Config &conf);

        static Options &getInstance();
    private:
        Options();
        /*TODO comment*/

        std::string inputFile_ = "";
        std::string outputFile_ = "";

        std::string engineFullpath_ = "/usr/bin/stockfish";
        int engineHashmapSize_ = 1024;
        int engineThreads_ = 2;

        int finderCutoffTreshold_ = 100;
        int playforMovetime_ = 1500;
        int playagainstMovetime_ = 1000;
        int verboseLevel_ = 0;
        int maxMoves_ = 255;

        int mateTreshold_ = 10000;
        int maxLines_ = 8;

        PositionList positions_;

        static Options instance_;
};


#endif
