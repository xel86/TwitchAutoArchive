SRC_DIR := src
OBJ_DIR := bin/obj
BIN_DIR := bin

EXE := $(BIN_DIR)/twitchautoarchive
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC := clang++
CPPFLAGS := -Iinclude -MMD -MP -std=c++17
CFLAGS   := -g -Wall
LDFLAGS  :=
LDLIBS   := -lcurl -lpthread

.PHONY: test clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
