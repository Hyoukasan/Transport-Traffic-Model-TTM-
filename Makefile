# ===================================
# Transport Traffic Model (TTM) on C
# ===================================

CC = gcc
OPT = -O2
CFLAGS = -Wall -Wextra -Ithird_party/include -Ithird_party/include/Other -g $(OPT)
LDFLAGS = -Lthird_party/lib
LIBS = -lglfw3 -lglew32 -lopengl32

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS = -lglfw -lGLEW -lGL
    LDFLAGS =
endif
ifeq ($(UNAME_S),Darwin)
    LIBS = -lglfw -lGLEW -framework OpenGL
    LDFLAGS =
endif

TARGET = TTM
BUILD_DIR = build
SRCS = src/application.c src/car.c src/graph.c src/main.c \
       src/menu.c src/renderer.c src/road_generator.c \
       src/texture.c src/geometry.c src/traffic_manager.c \
       src/audio_manager.c src/input.c src/debug_overlay.c

OBJECTS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@if exist "$(TARGET)" del /q "$(TARGET)"
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"

.PHONY: all clean
