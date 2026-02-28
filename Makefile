CARGS = -ggdb
OUTPUT = ./bin

pmain: src/main.c src/lexer.h
	$(CC) $(CARGS) src/main.c -o $(OUTPUT)/main
