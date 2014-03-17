/*
 * Oraclefinder, a program to find "perfect" chess game
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
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <memory>
#include <list>
#include <string>
#include <getopt.h>

#include "Thread.h"
#include "Engine.h"
#include "Utils.h"
#include "Output.h"
#include "Line.h"
#include "Finder.h"
#include "OracleFinder.h"
#include "MatfinderOptions.h"
#include "CommonMain.h"

using namespace std;



int main(int argc, char **argv)
{
    CommonMain::parseArgs(argc, argv);
    //The main object
    OracleFinder *theFinder = new OracleFinder();

    int retVal = CommonMain::theMain(argc, argv, theFinder);

    Out::output("Deleting Finder", 5);
    delete theFinder;
    return retVal;
}
