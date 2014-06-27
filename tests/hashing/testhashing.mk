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
ALL_TARGETS += testhashing
#INSTALL_TARGETS += install-boardTest
CLEAN_TARGETS += clean-testhashing
CHECK_TARGETS += check-testhashing


testhashing_SOURCES           := $(wildcard src/*.cpp)
testhashing_SOURCES           += $(wildcard tests/hashing/*.cpp)
testhashing_SOURCES_CXX       := $(wildcard tests/hashing/*.cxx)
testhashing_HEADERS_DEP       := $(wildcard include/*.h)
testhashing_HEADERS_DEP       += $(wildcard tests/hashing/*.h)

testhashing_OBJECTS := $(testhashing_SOURCES:.cpp=.o)
testhashing_OBJECTS += $(testhashing_SOURCES_CXX:.cxx=.o)


canonical_path := ../$(shell basename $(shell pwd -P))

#src/%.o: src/%.cpp $(boardTest_HEADERS_DEP)
	#echo "[boardTest] CXX $<"
	#$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

tests/hashing/polyglot_reference.o: tests/hashing/polyglot_reference.cpp tests/hashing/polyglot_reference.h
	echo "[Hashing Tester] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

tests/hashing/%.o: tests/hashing/%.cxx $(testhashing_HEADERS_DEP)
	echo "[Hashing Tester] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

testhashing: $(testhashing_OBJECTS)
	echo "[Hashing Tester] Link tester"
	$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

check-testhashing: testhashing tests/chessboard/input/testpositions.fen
	echo "[Hashing Tester] Check 1"
	./testhashing tests/chessboard/input/testpositions.fen

clean-testhashing:
	echo "[Hashing Tester] Clean"
	rm -f $(testhashing_OBJECTS) testhashing
