CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = final_project_demo
SOURCE = final_project_demo.c

all:
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

run: all
	./$(TARGET)

check:
	$(CC) $(CFLAGS) -fsyntax-only $(SOURCE)

clean:
	rm -f $(TARGET)

help:
	@echo "Available targets:"
	@echo "  make        - Build the project"
	@echo "  make run    - Build and run"
	@echo "  make check  - Check source syntax"
	@echo "  make clean  - Remove executable"
	@echo "  make help   - Show help"

