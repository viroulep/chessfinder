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
ALL_TARGETS += board
#INSTALL_TARGETS += install-boardTest
CLEAN_TARGETS += clean-boardTest

.SILENT:

boardTest_SOURCES           := $(wildcard src/*.cpp)
boardTest_SOURCES_CXX       := $(wildcard boardtest/*.cxx)
boardTest_HEADERS           := $(wildcard include/*.h)
boardTest_HEADERS_DEP       := $(wildcard include/*.h)

boardTest_OBJECTS := $(boardTest_SOURCES:.cpp=.o)
boardTest_OBJECTS += $(boardTest_SOURCES_CXX:.cxx=.o)


canonical_path := ../$(shell basename $(shell pwd -P))

#src/%.o: src/%.cpp $(boardTest_HEADERS_DEP)
	#echo "[boardTest] CXX $<"
	#$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

boardtest/%.o: boardtest/%.cxx $(boardTest_HEADERS_DEP)
	echo "[boardTest] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

board: $(boardTest_OBJECTS)
	echo "[boardTest] Link board"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

clean-boardTest:
	echo "[boardTest] Clean"
	rm -f $(boardTest_OBJECTS) board
