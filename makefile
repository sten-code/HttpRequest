# Compiler and Flags
CC = g++
CFLAGS = -Wall 

# Directories
SRC_DIR = examples
OBJ_DIR = examples/obj
BIN_DIR = examples/bin
VENDOR_DIR = vendor

# The name of the output
NAME = program

# All of the include directories and libraries
INCLUDES = -I$(SRC_DIR) -Iinclude -Ivendor/openssl/include
OPENSSL_DIR = $(VENDOR_DIR)/openssl
LIBDIRS = -L$(OPENSSL_DIR)
LIBS = -l:libssl.a -l:libcrypto.a

# The source and obj files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET = $(BIN_DIR)/$(NAME)

$(shell mkdir -p $(OBJ_DIR))
$(shell mkdir -p $(BIN_DIR))
$(shell mkdir -p $(VENDOR_DIR))

# Default target
all: $(TARGET)

# Build rules
$(OPENSSL_DIR):
	wget https://github.com/openssl/openssl/releases/download/openssl-3.2.1/openssl-3.2.1.tar.gz
	tar -xf openssl-*.tar.gz
	rm openssl-*.tar.gz
	mv openssl-* $(OPENSSL_DIR)
	cd $(OPENSSL_DIR); ./Configure; make \

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJ_FILES) $(OPENSSL_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@ $(LIBDIRS) $(LIBS)

# Clean the environment
clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(VENDOR_DIR)

run: $(TARGET)
	$(TARGET)
	
# Phony targets
.PHONY: clean run
