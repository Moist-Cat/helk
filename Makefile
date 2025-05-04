CC=clang
CXX=g++

CFLAGS=-g -Wall -Wextra
CXXFLAGS=-g -Wall -Wextra -Wno-self-assign

LEX_SOURCES=$(wildcard src/*.l) 
LEX_OBJECTS=$(patsubst %.l,%.c,${LEX_SOURCES}) $(patsubst %.l,%.h,${LEX_SOURCES})

YACC_SOURCES=$(wildcard src/*.y) 
YACC_OBJECTS=$(patsubst %.y,%.c,${YACC_SOURCES}) $(patsubst %.y,%.h,${YACC_SOURCES})

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES}) $(patsubst %.l,%.o,${LEX_SOURCES}) $(patsubst %.y,%.o,${YACC_SOURCES})
LIB_SOURCES=$(filter-out src/comp.c,${SOURCES})
LIB_OBJECTS=$(filter-out src/comp.o,${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c)
TEST_OBJECTS=$(filter-out tests/codegen_tests,$(patsubst %.c,%,${TEST_SOURCES}))
BUILTINS = src/builtins.c
BUILTINS_OBJ = src/builtins.o

LEX=flex
YACC=bison
YFLAGS?=-dv

# default to build a binary

all: build/libcomp.a build/comp ${OBJECTS}

run: build runtime
	./build/comp tests/make.hk > out.ll
	llc -filetype=obj out.ll -o out.o
	$(CC) out.o src/builtins.o -o out

execute: hulk compile
	$(CC) hulk/script.ll src/builtins.o -o hulk/script
	./hulk/script

runtime: $(HELPERS_OBJ)

# binaries

src/builtins.o: $(BUILTINS)
	$(CC) -c -o $@ $<

build/libcomp.a: ${LIB_OBJECTS} build_dir
	rm -f build/libcomp.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

src/comp.c: ${LEX_OBJECTS}

src/comp.o: src/comp.c build/libcomp.a
	${CC} ${CFLAGS} -c -o $@ $^

build/comp: ${OBJECTS}
	$(CC) $(CFLAGS) -rdynamic -Isrc -o $@ src/comp.o build/libcomp.a
	chmod 700 $@

build: build/comp
compile: build
	./hulk/comp script.hulk > ./hulk/script.ll

build_dir:
	mkdir -p build

hulk:
	mkdir -p hulk
	cd hulk && \
	ln -s ../build/comp comp


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
	rm -rf ${OBJECTS} ${LEX_OBJECTS} ${YACC_OBJECTS} build/ hulk/
