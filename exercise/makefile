CC=gcc
CFLAGS= -pthread -Wall -lm -O3

TARGET=hw1 hw2 hw3
ALL: $(TARGET)

$(TARGET): %: %.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -rf *.o $(TARGET) *.s
