#!/bin/bash
# Automatic dependency installation for TTM on Linux

echo "=== TTM Linux Dependencies Installer ==="

# Detect distribution
if command -v apt >/dev/null 2>&1; then
    DISTRO="debian"
    echo "Detected Debian/Ubuntu"
elif command -v dnf >/dev/null 2>&1; then
    DISTRO="fedora"
    echo "Detected Fedora/RHEL"
elif command -v pacman >/dev/null 2>&1; then
    DISTRO="arch"
    echo "Detected Arch Linux"
else
    echo "❌ Unsupported distribution"
    exit 1
fi

# Install dependencies
case $DISTRO in
    debian)
        echo "Installing dependencies for Debian/Ubuntu..."
        sudo apt update
        sudo apt install -y build-essential pkg-config
        sudo apt install -y libglfw3-dev libglew-dev libgl1-mesa-dev mesa-utils
        ;;
    fedora)
        echo "Installing dependencies for Fedora/RHEL..."
        sudo dnf install -y gcc make pkgconfig
        sudo dnf install -y glfw-devel glew-devel mesa-libGL-devel mesa-dri-drivers
        ;;
    arch)
        echo "Installing dependencies for Arch Linux..."
        sudo pacman -S --noconfirm gcc make pkgconf
        sudo pacman -S --noconfirm glfw glew mesa libgl
        ;;
esac

# Verify installation
echo "Verifying installation..."
echo -n "GCC: "
command -v gcc && gcc --version | head -1

echo -n "OpenGL: "
glxinfo 2>/dev/null | grep "OpenGL version" | head -1 || echo "Not available (may be Wayland)"

echo -n "GLFW: "
pkg-config --exists glfw3 && echo "Found" || echo "Not found"

echo -n "GLEW: "
pkg-config --exists glew && echo "Found" || echo "Not found"

echo "=== Installation Complete ==="
echo "Run: make -f Makefile.linux && ./TTM"

# После установки - сборка:
make clean && make