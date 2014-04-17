#
# Matfinder, a program to help chess engines to find mat
#
# Copyright© 2013 Philippe Virouleau
#
# You can contact me at firstname.lastname@imag.fr
# (Replace "firstname" and "lastname" with my actual names)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
ALL_TARGETS += testchessboard
#INSTALL_TARGETS += install-boardTest
CLEAN_TARGETS += clean-chessboardTest
CHECK_TARGETS += check-chessboardTest


chessboardTest_SOURCES           := $(wildcard src/*.cpp)
chessboardTest_SOURCES_CXX       := $(wildcard tests/chessboard/*.cxx)
chessboardTest_HEADERS_DEP       := $(wildcard include/*.h)

chessboardTest_OBJECTS := $(chessboardTest_SOURCES:.cpp=.o)
chessboardTest_OBJECTS += $(chessboardTest_SOURCES_CXX:.cxx=.o)


canonical_path := ../$(shell basename $(shell pwd -P))

#src/%.o: src/%.cpp $(boardTest_HEADERS_DEP)
	#echo "[boardTest] CXX $<"
	#$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

tests/chessboard/%.o: tests/chessboard/%.cxx $(chessboardTest_HEADERS_DEP)
	echo "[Chessboard Tester] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

testchessboard: $(chessboardTest_OBJECTS)
	echo "[Chessboard Tester] Link tester"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

check-chessboardTest: testchessboard tests/chessboard/input/testpositions.fen tests/chessboard/input/testpositions_2.fen
	echo "[Chessboard Tester] Check 1"
	./testchessboard tests/chessboard/input/testpositions.fen
	echo "[Chessboard Tester] Check 2"
	./testchessboard tests/chessboard/input/testpositions_2.fen

clean-chessboardTest:
	echo "[Chessboard Tester] Clean"
	rm -f $(chessboardTest_OBJECTS) testchessboard
