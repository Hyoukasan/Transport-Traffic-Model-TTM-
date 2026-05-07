# ===================================
# Transport Traffic Model (TTM) on C
# ===================================

CC = gcc
OPT = -O2
CFLAGS = -Wall -Wextra -DGLEW_NO_GLU -DGLFW_INCLUDE_NONE -Ithird_party/include -Ithird_party/include/Other -g $(OPT)
LDFLAGS = -Lthird_party/lib
LIBS = -lglfw3 -lglew32 -lopengl32
PREPARE_LIBS =
MKDIR_BUILD = @if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
CLEAN_CMD = @if exist "$(TARGET)" del /q "$(TARGET)"
CLEAN_BUILD = @if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"

# Detect OS
ifeq ($(OS),Windows_NT)
    UNAME_S = Windows_NT
else
    UNAME_S := $(shell uname -s)
endif
ifeq ($(UNAME_S),Linux)
    PREPARE_LIBS = prepare-linux-libs
    LIBS = -lglfw -lGLEW -l:libGL.so.1 -lm
    LDFLAGS = -Lthird_party/lib/linux -Wl,-rpath,'$$ORIGIN/third_party/lib/linux'
    MKDIR_BUILD = mkdir -p "$(BUILD_DIR)"
    CLEAN_CMD = rm -f "$(TARGET)"
    CLEAN_BUILD = rm -rf "$(BUILD_DIR)"
endif
ifeq ($(UNAME_S),Darwin)
    LIBS = -lglfw -lGLEW -framework OpenGL
    LDFLAGS =
    MKDIR_BUILD = mkdir -p "$(BUILD_DIR)"
    CLEAN_CMD = rm -f "$(TARGET)"
    CLEAN_BUILD = rm -rf "$(BUILD_DIR)"
endif

TARGET = TTM
BUILD_DIR = build
SRCS = src/application.c src/car.c src/graph.c src/main.c \
       src/menu.c src/renderer.c src/road_generator.c \
       src/texture.c src/geometry.c src/traffic_manager.c \
       src/audio_manager.c src/input.c src/debug_overlay.c \
       src/config_manager.c

OBJECTS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

all: $(PREPARE_LIBS) $(BUILD_DIR) $(TARGET)

prepare-linux-libs:
	ln -sf libglfw.so.3.3 third_party/lib/linux/libglfw.so.3
	ln -sf libGLEW.so.2.2.0 third_party/lib/linux/libGLEW.so.2.2

$(BUILD_DIR):
	$(MKDIR_BUILD)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(CLEAN_CMD)
	$(CLEAN_BUILD)

.PHONY: all clean prepare-linux-libs
