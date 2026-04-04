# Transport Traffic Model (TTM) - Linux Installation Guide

## OpenGL на Linux

### Важно знать про OpenGL:
- **OpenGL обычно уже установлен** на большинстве Linux дистрибутивов
- Он идет вместе с драйверами видео (mesa, nvidia, amd)
- **Нужны только заголовочные файлы** для компиляции

### Проверка наличия OpenGL:
```bash
# Проверка версии OpenGL
glxinfo | grep "OpenGL version"

# Проверка заголовочных файлов
ls -la /usr/include/GL/

# В программе TTM будет выведена информация:
# OpenGL Version: 4.6 (Compatibility Profile) Mesa 21.2.6
# Renderer: llvmpipe (LLVM 12.0.1, 256 bits)
# Vendor: Mesa/X.org
```

### Установка OpenGL (если отсутствует):
```bash
# Ubuntu/Debian
sudo apt install -y mesa-utils libgl1-mesa-dev

# Fedora
sudo dnf install -y mesa-libGL-devel mesa-dri-drivers

# Arch Linux
sudo pacman -S --noconfirm mesa libgl

# Для NVIDIA
sudo apt install -y nvidia-driver nvidia-glx

# Для AMD
sudo apt install -y mesa-vulkan-drivers
```

### Что делает программа:
1. **Проверяет OpenGL при запуске** и выводит версию
2. **Если OpenGL нет** - программа завершится с ошибкой
3. **Если есть, но старая версия** - может работать в compatibility mode

## Быстрый старт

### Графическая версия (рекомендуется)
```bash
# Установка зависимостей
sudo apt update
sudo apt install -y build-essential libglfw3-dev libglew-dev libgl1-mesa-dev

# Сборка и запуск
```bash
# Установка зависимостей
chmod +x linux_install_deps.sh
./linux_install_deps.sh

# Тест OpenGL
make -f Makefile.linux opengl-test
./opengl_test

# Сборка основной программы
make -f Makefile.linux
./TTM
```

## Режимы работы

### 1. Полная графическая версия
```bash
make -f Makefile.linux
./TTM
```

### 2. Headless версия (без графики)
```bash
make -f Makefile.linux headless
./TTM_headless
```

### 3. OpenGL тест
```bash
make -f Makefile.linux opengl-test
./opengl_test
```

## Для "голого" Linux (минимальная установка)

### Шаг 1: Установка базовых инструментов
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y build-essential pkg-config

# Fedora/CentOS
sudo dnf install -y gcc make pkgconfig
# или
sudo yum install -y gcc make pkgconfig

# Arch Linux
sudo pacman -S --noconfirm gcc make pkgconf
```

### Шаг 2: Установка графических библиотек
```bash
# Ubuntu/Debian
sudo apt install -y libglfw3-dev libglew-dev libgl1-mesa-dev libx11-dev

# Fedora/CentOS
sudo dnf install -y glfw-devel glew-devel mesa-libGL-devel libX11-devel
# или
sudo yum install -y glfw-devel glew-devel mesa-libGL-devel libX11-devel

# Arch Linux
sudo pacman -S --noconfirm glfw glew mesa libx11
```

### Шаг 3: Сборка и запуск
```bash
# Используем Linux-специфичный Makefile
make -f Makefile.linux clean
make -f Makefile.linux

# Запуск
./TTM
```

## Возможные проблемы

### 1. "GLFW3 not found"
```bash
# Ubuntu/Debian - альтернативные пакеты
sudo apt install -y libglfw3 libglfw3-dev

# Или собрать из исходников
wget https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip
unzip glfw-3.3.8.zip
cd glfw-3.3.8
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON
make && sudo make install
```

### 2. "GLEW not found"
```bash
# Ubuntu/Debian
sudo apt install -y libglew-dev

# Или собрать из исходников
wget https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip
unzip glew-2.2.0.zip
cd glew-2.2.0
make && sudo make install
```

### 3. Проблемы с OpenGL
```bash
# Установка драйверов
sudo apt install -y mesa-utils

# Проверка OpenGL
glxinfo | grep "OpenGL version"
```

### 4. Проблемы с X11
```bash
# Установка X11 headers
sudo apt install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

## Альтернативный вариант: Wayland

Если система использует Wayland вместо X11:
```bash
# Установка Wayland зависимостей
sudo apt install -y libwayland-dev libxkbcommon-dev

# GLFW с Wayland поддержкой
sudo apt install -y libglfw3-wayland libglfw3-wayland-dev
```

## Тестирование

После установки запустите:
```bash
make -f Makefile.linux run
```

Если окно не открывается, проверьте:
1. Графическое окружение (X11/Wayland)
2. Переменные DISPLAY (для X11)
3. Драйверы видео

## Совместимость

Протестировано на:
- Ubuntu 20.04+
- Fedora 35+
- Arch Linux
- Debian 11+

Требования:
- GCC 9+
- Linux kernel 4.15+
- X11 или Wayland
- OpenGL 3.3+