This repository hosts a pair of tools used to help chess engines.

# Dependencies

You need the following dependendies :

* The pthread library
* A C++11 compiler (such as GCC 4.9 or Clang 3.5)
* Stockfish or an UCI engine

It has been tested on Unix and Mac.

# Matfinder

## Overview

The aim of matfinder is to close an unbalanced positions by mates.
It will explore the best line deep first until it finds a mate, and then backtrack until it finds that the best line is not mate, and re-iterate this process until the best line is mate.

This works because the chess engine does keep the known line in its memory. If you doesn't set correctly (ie: high enough) the amount of memory used by the chess engine, then matfinder may not terminate.

I have been able to close the whole 5x5 gardner minichess in a single execution (ie: prove that 1.c4, 1.Nb4, 1.Nd4 are mate for black) with 8GB memory given to stockfish.

More details on the whole resolution (including oracles for 1.b4, 1.d4, 1.e4, 1.f4) can be found here <http://membres-lig.imag.fr/prost/MiniChessResolution/>.



## Usage
See `./matfinder -h`, some example usage can be found there and in the scripts in fens/gardner (you need the modified stockfish version from the above link).

# Oraclefinder

## Overview

Oraclefinder can be used to build oracles of "perfect" games. Since it's still a WIP, I won't go any further in the details, but I will update this section as soon as we have a working version of this program.


# Acknowledgements

This program has been regularly improved thanks to the help of Mehdi Mhalla and Frédéric Prost.
