CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -std=c99
CXXFLAGS := -Wall -Wextra -std=c++17
INCLUDES := -Iinclude
SRC_DIR := src/callbacks
OBJ_DIR := build
TARGET := $(OBJ_DIR)/callback_manager.o
TEST_DIR := test
TEST_SRC := $(TEST_DIR)/callback_manager_test.c
TEST_BIN := $(OBJ_DIR)/test_callback_manager
CPP_EXAMPLE_SRC := examples/callback_example.cpp
CPP_EXAMPLE_BIN := $(OBJ_DIR)/callback_example_cpp

.PHONY: all test cpp_example clean

all: $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_DIR)/callback_manager.c -o $(TARGET)

$(TEST_BIN): | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SRC) $(SRC_DIR)/callback_manager.c -o $(TEST_BIN)

$(CPP_EXAMPLE_BIN): | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CPP_EXAMPLE_SRC) $(SRC_DIR)/callback_manager.c -o $(CPP_EXAMPLE_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

cpp_example: $(CPP_EXAMPLE_BIN)
	@echo "Built C++ example at $(CPP_EXAMPLE_BIN)"

clean:
	rm -rf $(OBJ_DIR)
