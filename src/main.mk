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
ALL_TARGETS += outCxx matfinder oraclefinder
INSTALL_TARGETS += install-matfinder
CLEAN_TARGETS += clean-matfinder

.PHONY: outCxx

.SILENT:


common_SOURCES           := $(wildcard src/*.cpp)
#matfinder_SOURCES       := $(wildcard src/matfinder/*.cxx)
#oraclefinder_SOURCES       := $(wildcard src/oraclefinder/*.cxx)
finder_HEADERS_DEP       := $(wildcard include/*.h)

finder_OBJECTS := $(common_SOURCES:.cpp=.o)
#matfinder_OBJECTS += $(matfinder_SOURCES:.cxx=.o)

#oraclefinder_OBJECTS := $(common_SOURCES:.cpp=.o)
#oraclefinder_OBJECTS += $(oraclefinder_SOURCES:.cxx=.o)

canonical_path := ../$(shell basename $(shell pwd -P))

outCxx:
	echo "Using Cxx="$(CXX)

#TODO
src/%.o: src/%.cpp $(finder_HEADERS_DEP)
	echo "[matfinder] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

src/%.o: src/%.cxx $(finder_HEADERS_DEP)
	echo "[matfinder] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

src/main_oracle.o: src/main.cxx $(finder_HEADERS_DEP)
	echo "[oraclefinder] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -DORACLEFINDER -c -o $@ ${canonical_path}/$<

src/main_finder.o: src/main.cxx $(finder_HEADERS_DEP)
	echo "[matfinder] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -DMATFINDER -c -o $@ ${canonical_path}/$<

oraclefinder: $(finder_OBJECTS) src/main_oracle.o
	echo "[oraclefinder] Link oraclefinder"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

matfinder: $(finder_OBJECTS) src/main_finder.o
	echo "[matfinder] Link matfinder"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

clean-matfinder:
	echo "[matfinder] Clean"
	rm -f $(matfinder_OBJECTS) $(oraclefinder_OBJECTS) matfinder oraclefinder
