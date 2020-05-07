#pragma once
#define GL_SILENCE_DEPRECATION 1

#include <OpenGL/gl3.h>

GLuint shader_load(const char vertex_file_path[],
                   const char fragment_file_path[]);
