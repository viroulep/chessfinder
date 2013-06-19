ALL_TARGETS += matfinder-gardner
INSTALL_TARGETS += install-gardner
CLEAN_TARGETS += clean-gardner

.SILENT:

gardner_SOURCES           := $(wildcard src/*.cpp)
gardner_HEADERS           := $(wildcard include/*.h)
gardner_HEADERS_DEP           := $(wildcard include/*.h)

gardner_OBJECTS := $(gardner_SOURCES:.cpp=.o)


canonical_path := ../$(shell basename $(shell pwd -P))

src/%.o: src/%.cpp $(gardner_HEADERS_DEP)
	echo "[gardner] CXX $<"
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ ${canonical_path}/$<

matfinder-gardner: $(gardner_OBJECTS)
	echo "[gardner] Link gardner"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

clean-gardner:
	echo "[gardner] Clean"
	rm -f $(gardner_OBJECTS) matfinder-gardner
