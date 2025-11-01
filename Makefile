# =========================
# Reed–Solomon File Codec
# =========================

# Compiler and flags
CXX       := g++
CXXFLAGS  := -std=c++11 -O3 -Wall -Wextra -I./
LDFLAGS   := 

# Output directories
BUILD_DIR := build
SRC_DIR   := .
BIN_DIR   := .

# Source files
SRCS := $(SRC_DIR)/rs_file_encode.cpp \
        $(SRC_DIR)/rs_file_decode.cpp

# Object files (in build/)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executables
EXE_ENC := $(BIN_DIR)/rs_file_encode
EXE_DEC := $(BIN_DIR)/rs_file_decode

# Default target
all: $(EXE_ENC) $(EXE_DEC)

# Build rules
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXE_ENC): $(BUILD_DIR)/rs_file_encode.o
	@echo "Linking $@..."
	$(CXX) $^ -o $@ $(LDFLAGS)

$(EXE_DEC): $(BUILD_DIR)/rs_file_decode.o
	@echo "Linking $@..."
	$(CXX) $^ -o $@ $(LDFLAGS)

# Create build directory if needed
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Housekeeping
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(EXE_ENC) $(EXE_DEC)

# Convenience targets
encode: $(EXE_ENC)
decode: $(EXE_DEC)

# Optional test run
test: $(EXE_ENC) $(EXE_DEC)
	@echo "Running test sequence..."
	./rs_file_encode input.txt
	cp input.txt input_corrupt.txt
	printf '\x00\xFF' | dd of=input_corrupt.txt bs=1 seek=10 count=2 conv=notrunc
	./rs_file_decode input_corrupt.txt file.parity
	sha256sum input.txt recovered.txt

.PHONY: all clean encode decode test
