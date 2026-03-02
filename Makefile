CARGS = -ggdb
LARGS = -lm
OUTPUT = ./bin

pmain: src/main.c src/lexer.h
	mkdir -p $(OUTPUT) && $(CC) $(CARGS) src/main.c -o $(OUTPUT)/main $(LARGS)
