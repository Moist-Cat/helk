CC=clang

CFLAGS=-lm -g -Wall -Wextra -fsanitize=address,undefined
CFLAGS=-lm -g -Wall -Wextra

LP_SOURCES=src/lexer.helk src/lexer.helk
LP_OBJECTS=src/lexer.h src/lexer.c src/parser.h src/parser.c src/regex_dfa.h src/regex_dfa.c

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES}) $(patsubst %.helk,%.o,${LP_SOURCES})
LIB_SOURCES=$(filter-out src/comp.c,${SOURCES})
LIB_OBJECTS=$(filter-out src/comp.o,${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c)
TEST_OBJECTS=$(filter-out tests/codegen_tests,$(patsubst %.c,%,${TEST_SOURCES}))
BUILTINS = src/builtins.c
BUILTINS_OBJ = src/builtins.o

# :p
LP=python src/main.py

# default to build a binary

all: build/libcomp.a build/comp ${OBJECTS}

run: build runtime
	./build/comp tests/make.hk > out.ll
	llc -filetype=obj out.ll -o out.o
	${CC} ${CFLAGS} out.o src/builtins.o -o out

execute: compile
	${CC} ${CFLAGS} hulk/script.ll src/builtins.o -o hulk/script && hulk/script

runtime: $(HELPERS_OBJ)

# binaries

src/builtins.o: $(BUILTINS)
	${CC} ${CFLAGS} -c -o $@ $<

build/libcomp.a: ${LIB_OBJECTS} build_dir
	rm -f build/libcomp.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

src/comp.c: ${LEX_OBJECTS}

src/comp.o: src/comp.c build/libcomp.a
	${CC} ${CFLAGS} -c -o $@ $^

build/comp: ${OBJECTS}
	${CC} ${CFLAGS} -rdynamic -Isrc -o $@ src/comp.o build/libcomp.a
	chmod 700 $@

build: build/comp
compile: hulk build
	./hulk/comp script.hulk > ./hulk/script.ll

build_dir:
	mkdir -p build

hulk:
	mkdir -p hulk
	cd hulk && \
	ln -s ../build/comp comp


# flex & bison

src/lexer.c: src/parser.c
	${LP}

src/parser.c: src/parser.helk
	${LP}


src/codegen.o: src/codegen.c
	${CC} ${LLVM_CC_FLAGS} ${CFLAGS} -c -o $@ $^
	

# clean

clean: 
	rm -rf ${OBJECTS} ${LP_OBJECTS} build/ hulk/ src/lexer.c src/parser.c src/regex_dfa.c
