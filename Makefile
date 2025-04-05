CC = gcc
LEX = flex
YACC = bison
LLVM_CONFIG = llvm-config

CFLAGS = -Wall -g -MMD `$(LLVM_CONFIG) --cflags`
LDFLAGS = `$(LLVM_CONFIG) --ldflags --libs --system-libs`

SRC_DIR = src
OBJ_DIR = obj
DEP_DIR = deps

PROGRAM = comp

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = src/parser.o src/lexer.o $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(patsubst $(SRC_DIR)/%.c, $(DEP_DIR)/%.d, $(SRCS))

# Ensure the obj and deps directories exist
$(shell mkdir -p $(OBJ_DIR) $(DEP_DIR))

all: $(PROGRAM)
build: $(PROGRAM)
run:
	echo "sabre"

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $^ $(LDFLAGS)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(SRC_DIR)/parser.c: $(SRC_DIR)/parser.y
	$(YACC) src/parser.y --defines=src/parser.h -o src/parser.c


$(SRC_DIR)/lexer.c: $(SRC_DIR)/lexer.l
	$(LEX) -o src/lexer.c src/lexer.l

# bison src/parser.y --defines=src/parser.h -o src/parser.c && flex -o src/lexer.c src/lexer.l

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(DEP_DIR) compiler $(SRC_DIR)/lexer.c $(SRC_DIR)/parser.c $(SRC_DIR)/parser.h

