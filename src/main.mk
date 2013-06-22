ALL_TARGETS += matfinder
INSTALL_TARGETS += install-matfinder
CLEAN_TARGETS += clean-matfinder

.SILENT:

matfinder_SOURCES           := $(wildcard src/*.cpp)
matfinder_HEADERS           := $(wildcard include/*.h)
matfinder_HEADERS_DEP           := $(wildcard include/*.h)

matfinder_OBJECTS := $(matfinder_SOURCES:.cpp=.o)


canonical_path := ../$(shell basename $(shell pwd -P))

src/%.o: src/%.cpp $(matfinder_HEADERS_DEP)
	echo "[matfinder] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

matfinder: $(matfinder_OBJECTS)
	echo "[matfinder] Link matfinder"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

clean-matfinder:
	echo "[matfinder] Clean"
	rm -f $(matfinder_OBJECTS) matfinder
