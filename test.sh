llc -filetype=obj out.ll -o out.o && clang -lm out.o  src/builtins.o -o out
