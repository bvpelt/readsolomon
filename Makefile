# =========================
# Reed–Solomon File Codec
# =========================

# Compiler and flags
CXX       := g++
CXXFLAGS  := -std=c++11 -O3 -Wall -Wextra -I./
#CXXFLAGS  := -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -DDEBUG -I./
LDFLAGS   := 

# Output directories
BUILD_DIR := build
SRC_DIR   := .
BIN_DIR   := .

# Source files
SRCS := $(SRC_DIR)/rs_file_encode.cpp \
        $(SRC_DIR)/rs_file_decode.cpp \
		$(SRC_DIR)/rs_codec.cpp \
		$(SRC_DIR)/reed.cpp

# Object files (in build/)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executables
EXE_ENC := $(BIN_DIR)/rs_file_encode
EXE_DEC := $(BIN_DIR)/rs_file_decode
EXE_MAIN := $(BIN_DIR)/reed

# Default target
all: $(EXE_ENC) $(EXE_DEC) $(EXE_MAIN)

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

$(EXE_MAIN): $(BUILD_DIR)/reed.o $(BUILD_DIR)/rs_codec.o
	@echo "Linking $@..."
	$(CXX) $^ -o $@ $(LDFLAGS)

# Create build directory if needed
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Housekeeping
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(EXE_ENC) $(EXE_DEC) $(EXE_MAIN)

# Generate listing
# Rebuild with preprocessed listings (macros expanded)
rebuild-listing: clean
	@echo "Rebuilding with preprocessed listings..."
	mkdir -p $(BUILD_DIR)/listing
	@for src in $(SRCS); do \
	    base=$$(basename $$src .cpp); \
	    echo "Generating listing for $$src → $(BUILD_DIR)/listing/$$base.i"; \
	    $(CXX) $(CXXFLAGS) -E -P $$src -o $(BUILD_DIR)/listing/$$base.i; \
	done
	@echo "Preprocessed listings stored in $(BUILD_DIR)/listing/"

#
# Generate asm listing replace above $(CXX) command with:
# $(CXX) $(CXXFLAGS) -g -O2 -fverbose-asm -Wa,-adhln -S $$src -o $(BUILD_DIR)/listing/$$base.i; \	

# Convenience targets
encode: $(EXE_ENC)
decode: $(EXE_DEC)
reed: $(EXE_MAIN)

# Optional test run
test: $(EXE_ENC) $(EXE_DEC) $(EXE_MAIN)
	@echo "Running test sequence..."
	./rs_file_encode input.txt
	cp input.txt input_corrupt.txt
	printf '\x00\xFF' | dd of=input_corrupt.txt bs=1 seek=10 count=2 conv=notrunc
	./rs_file_decode input_corrupt.txt file.parity
	sha256sum input.txt recovered.txt
	./reed -e input.txt
	cp input.txt input_corrupt.txt
	printf '\x00\xFF' | dd of=input_corrupt.txt bs=1 seek=10 count=2 conv=notrunc
	./reed -d input.txt
	sha256sum input.txt input.txt.recovered


.PHONY: all clean encode decode reed test rebuild_listing
