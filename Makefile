# ===================================
# Transport Traffic Model (TTM) on C
# ===================================

CC = gcc
OPT = -O2
CFLAGS = -Wall -Wextra -Ithird_party/include -g $(OPT)
LDFLAGS = -Lthird_party/lib
LIBS = -lglfw3 -lglew32 -lopengl32
TARGET = TTM.exe
BUILD_DIR = build
SRCS = $(wildcard src/*.c)
OBJECTS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) $(OBJECTS)