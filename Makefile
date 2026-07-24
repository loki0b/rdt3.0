INCLUDE = include
SRC = $(wildcard src/*.c src/*/*.c)
OUT = app
OUT_DIR = bin

all:
	mkdir -p $(OUT_DIR)
	gcc -Wall $(SRC) -o $(OUT_DIR)/$(OUT)

run:
	./$(OUT_DIR)/$(OUT)

clean:
	rm ./$(OUT_DIR)/$(OUT)