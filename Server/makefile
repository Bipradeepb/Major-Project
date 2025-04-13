# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Iincludes -std=c++17 -MMD -MP -O3 # Add flags for dependency generation
BUILD_DIR := build
SRC_DIR := src
TARGET := $(BUILD_DIR)/ser_exe

# Find all .cpp files in src directory and its subdirectories
SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
# Create corresponding .o file paths in the build directory
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
DEP_FILES := $(OBJ_FILES:.o=.d)  # Corresponding .d files

# Default rule
all: $(TARGET)

# Rule to create the target executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile each .cpp file into a .o file in the build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build option
debug: CXXFLAGS += -g
debug: all

# Clean up all generated files
clean:
	rm -rf $(BUILD_DIR)

# Include dependency files, if they exist
-include $(DEP_FILES)

# PHONY targets to avoid conflicts with file names
.PHONY: all clean debug
