# Compiler and Flags
CC = g++
CFLAGS = -Wall 

# Directories
SRC_DIR = examples
OBJ_DIR = examples/obj
BIN_DIR = examples/bin

# The name of the output
NAME = program

# All of the include directories and libraries
INCLUDES = -I$(SRC_DIR) -Iinclude -Ilib/openssl/include
LIBDIR = -Llib/openssl
LIBS = -l:libssl.a -l:libcrypto.a

# The source and obj files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET = $(BIN_DIR)/$(NAME)

$(shell mkdir -p $(OBJ_DIR))
$(shell mkdir -p $(BIN_DIR))

# Default target
all: $(TARGET)

# Build rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBDIR) $(LIBS)

# Clean the environment
clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)

run: $(TARGET)
	$(TARGET)
	

# Phony targets
.PHONY: clean run
