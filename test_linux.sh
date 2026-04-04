#!/bin/bash
# Test script for Linux compatibility

echo "=== TTM Linux Compatibility Test ==="

# Check if we're on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "❌ This test is for Linux systems only"
    exit 1
fi

echo "✓ Running on Linux"

# Check basic tools
echo -n "Checking GCC... "
if command -v gcc >/dev/null 2>&1; then
    echo "✓ GCC found: $(gcc --version | head -1)"
else
    echo "❌ GCC not found"
    exit 1
fi

echo -n "Checking Make... "
if command -v make >/dev/null 2>&1; then
    echo "✓ Make found"
else
    echo "❌ Make not found"
    exit 1
fi

# Check dependencies
echo -n "Checking pkg-config... "
if command -v pkg-config >/dev/null 2>&1; then
    echo "✓ pkg-config found"
else
    echo "❌ pkg-config not found"
fi

echo -n "Checking GLFW3... "
if pkg-config --exists glfw3 2>/dev/null; then
    echo "✓ GLFW3 found: $(pkg-config --modversion glfw3)"
else
    echo "❌ GLFW3 not found"
fi

echo -n "Checking OpenGL headers... "
if [[ -f "/usr/include/GL/gl.h" ]] || [[ -f "/usr/local/include/GL/gl.h" ]]; then
    echo "✓ OpenGL headers found"
else
    echo "❌ OpenGL headers not found"
fi

echo -n "Checking OpenGL runtime... "
if command -v glxinfo >/dev/null 2>&1; then
    OPENGL_VERSION=$(glxinfo 2>/dev/null | grep "OpenGL version" | head -1)
    if [[ -n "$OPENGL_VERSION" ]]; then
        echo "✓ $OPENGL_VERSION"
    else
        echo "❌ glxinfo found but no OpenGL"
    fi
else
    echo "⚠️ glxinfo not found (may be Wayland or no X11)"
fi

# Try to compile
echo "Testing compilation..."
if make -f Makefile.linux clean >/dev/null 2>&1; then
    echo "✓ Clean successful"
else
    echo "❌ Clean failed"
fi

if make -f Makefile.linux >/dev/null 2>&1; then
    echo "✓ Compilation successful"
    if [[ -f "TTM" ]]; then
        echo "✓ Executable created: $(ls -lh TTM)"
    else
        echo "❌ Executable not found"
    fi
else
    echo "❌ Compilation failed"
    make -f Makefile.linux 2>&1 | head -10
fi

echo "=== Test Complete ==="
echo "If compilation failed, install dependencies:"
echo "  Ubuntu/Debian: sudo apt install libglfw3-dev libglew-dev libgl1-mesa-dev"
echo "  Fedora: sudo dnf install glfw-devel glew-devel mesa-libGL-devel"