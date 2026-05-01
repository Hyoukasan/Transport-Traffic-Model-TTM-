#ifndef TEXTURE_H
#define TEXTURE_H

unsigned int texture_load(const char *path, int *out_width, int *out_height);
void         texture_delete(unsigned int tex_id);

#endif