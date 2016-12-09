
CXX_FILES := test.cxx tree_bitmap.cxx
OBJECTS := $(CXX_FILES:.cxx=.o)

CXX_FLAGS := -std=c++11 -Wall -lpthread -O2
LINK_FLAGS := -pthread -O2

test: $(OBJECTS)
	@$(CXX) $^ -o $@ $(LINK_FLAGS)

%.o: %.cxx
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	@rm -f test *.o
