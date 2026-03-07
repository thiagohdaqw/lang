CARGS = -ggdb
LARGS = -lm

OUTPUT = ./bin
BUILD = ./.pypt_build

pmain: src/main.c src/lexer.h
	mkdir -p $(OUTPUT) && $(CC) $(CARGS) src/main.c -o $(OUTPUT)/main $(LARGS)

clean:
	rm -rf $(OUTPUT) $(BUILD)