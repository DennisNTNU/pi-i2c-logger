SRC = main.c src/si7021.c src/tmp102.c src/log_local.c
TARGET_DIR = build
TARGET = $(TARGET_DIR)/main

$(TARGET): $(SRC)
	mkdir -p build
	gcc -o $@ $^ -Wall -Iinc