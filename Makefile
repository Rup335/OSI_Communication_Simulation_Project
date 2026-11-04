CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lws2_32
BIN_DIR = bin
SRC_DIR = src

all: $(BIN_DIR)/server.exe $(BIN_DIR)/client.exe

$(BIN_DIR):
	if not exist $(BIN_DIR) mkdir $(BIN_DIR)

$(BIN_DIR)/server.exe: $(SRC_DIR)/server.c $(SRC_DIR)/layers.c $(SRC_DIR)/layers.h $(SRC_DIR)/osi_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/server.c $(SRC_DIR)/layers.c -o $(BIN_DIR)/server.exe $(LDFLAGS)

$(BIN_DIR)/client.exe: $(SRC_DIR)/client.c $(SRC_DIR)/layers.c $(SRC_DIR)/layers.h $(SRC_DIR)/osi_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/client.c $(SRC_DIR)/layers.c -o $(BIN_DIR)/client.exe $(LDFLAGS)

clean:
	if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)

.PHONY: all clean
