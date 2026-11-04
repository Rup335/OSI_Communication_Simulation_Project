CC = gcc
CFLAGS = -Wall -O2

osi.exe: main.c ../src/layers.c
	$(CC) $(CFLAGS) main.c ../src/layers.c -o osi.exe -lws2_32

clean:
	if exist *.o del /f /q *.o
	if exist osi.exe del /f /q osi.exe

.PHONY: clean
