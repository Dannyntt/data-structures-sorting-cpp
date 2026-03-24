CXX      = g++
CXXFLAGS = -O2 -std=c++17
TARGET   = sorter
SRC      = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) dataset.txt

clean:
	rm -f $(TARGET)

.PHONY: all run clean
