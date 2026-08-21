CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = processflow
SRC = processflow.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

test: $(TARGET)
	@echo "Executando teste basico do ProcessFlow..."
	@echo "exit" | ./$(TARGET)

.PHONY: all clean test