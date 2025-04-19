CC=gcc
CXX=g++

CFLAGS=-g -Wall -Wextra
CXXFLAGS=-g -Wall -Wextra -Wno-self-assign

LEX_SOURCES=$(wildcard src/*.l) 
LEX_OBJECTS=$(patsubst %.l,%.c,${LEX_SOURCES}) $(patsubst %.l,%.h,${LEX_SOURCES})

YACC_SOURCES=$(wildcard src/*.y) 
YACC_OBJECTS=$(patsubst %.y,%.c,${YACC_SOURCES}) $(patsubst %.y,%.h,${YACC_SOURCES})

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES}) $(patsubst %.l,%.o,${LEX_SOURCES}) $(patsubst %.y,%.o,${YACC_SOURCES})
LIB_SOURCES=$(filter-out comp.c,${SOURCES})
LIB_OBJECTS=$(filter-out comp.o,${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c)
TEST_OBJECTS=$(filter-out tests/codegen_tests,$(patsubst %.c,%,${TEST_SOURCES}))

LEX=flex
YACC=bison
YFLAGS?=-dv

LLVM_CC_FLAGS=`llvm-config --cflags`
LLVM_LINK_FLAGS=`llvm-config --libs --cflags --ldflags core analysis executionengine mcjit interpreter native --system-libs`

# default to build a binary

all: build/libcomp.a build/comp ${OBJECTS}


# binaries

build/libcomp.a: build_dir ${LIB_OBJECTS}
	rm -f build/libcomp.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

src/comp.c: ${LEX_OBJECTS}

src/comp.o: src/comp.c
	${CC} ${LLVM_CC_FLAGS} ${CFLAGS} -c -o $@ $^

build/comp: ${OBJECTS}
	$(CXX) $(LLVM_LINK_FLAGS) $(CXXFLAGS) -rdynamic -Isrc -o $@ src/comp.o build/libcomp.a
	chmod 700 $@

build: build/comp

build_dir:
	mkdir -p build


# flex & bison

src/lexer.c: src/parser.c
	${LEX} --header-file=src/lexer.h -o $@ src/lexer.l

src/parser.c: src/parser.y
	mkdir -p build/bison
	${YACC} ${YFLAGS} -o $@ $^


# llvm

src/codegen.o: src/codegen.c
	${CC} ${LLVM_CC_FLAGS} ${CFLAGS} -c -o $@ $^
	

# clean

clean: 
	rm -rf comp ${OBJECTS} ${LEX_OBJECTS} ${YACC_OBJECTS}
