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
