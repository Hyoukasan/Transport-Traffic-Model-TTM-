#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int texture_load(const char *path, int *out_width, int *out_height) {
    if (path == NULL) {
        return 0;
    }

    int width, height, channels;
    unsigned char *pixels = stbi_load(path, &width, &height, &channels, 4);
    if (pixels == NULL) {
        fprintf(stderr, "texture_load: failed to load image '%s'\n", path);
        return 0;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixels);

    if (out_width) {
        *out_width = width;
    }
    if (out_height) {
        *out_height = height;
    }

    return (unsigned int)texture_id;
}

void texture_delete(unsigned int tex_id) {
    if (tex_id == 0) {
        return;
    }

    GLuint texture = (GLuint)tex_id;
    glDeleteTextures(1, &texture);
}
