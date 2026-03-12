CARGS = -ggdb
LARGS = -lm

OUTPUT = ./bin
BUILD = ./.pypt_build

all: build_folder main stdlib

main: build_folder src/main.c src/lexer.h
	$(CC) $(CARGS) src/main.c -o $(OUTPUT)/main $(LARGS)

stdlib: build_folder main stdlib/io.pypt
	$(OUTPUT)/main stdlib/io.pypt -C && cp $(BUILD)/out.o $(BUILD)/stdlib/io.o

build_folder:
	mkdir -p $(BUILD)/stdlib $(OUTPUT)

clean:
	rm -rf $(OUTPUT) $(BUILD)
