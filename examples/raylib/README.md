## Instruções de uso

1. Baixe o raylib em https://github.com/raysan5/raylib/releases/tag/5.5 e o extraia em uma pasta
2. Builde o compilador `make`
3. Builde o exemplo `janela.pypt`
```
$ ./bin/main examples/raylib/janela.pypt -L "-L../raylib/raylib-5.5_linux_amd64/lib/ -Wl,-rpath='../raylib/raylib-5.5_linux_amd64/lib/' -lraylib -lm"
```
4. Execute o programa
```
$ ./main
```
