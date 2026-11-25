CXX = g++
CXXFLAGS = -O2 -std=c++17

TARGET = grid_pathfinding

all: $(TARGET)

$(TARGET): grid_pathfinding.cpp algorithms/brute_force.cpp algorithms/dynamic_programming.cpp algorithms/greedy.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(TARGET).exe

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
