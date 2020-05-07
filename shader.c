#include "shader.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "malloc.h"
#include "types.h"

GLuint shader_load(const char vertex_file_path[],
                   const char fragment_file_path[]) {
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    const usize vertex_shader_src_capacity = 5000;
    u8* vertex_shader_src = ogl_malloc(vertex_shader_src_capacity);
    usize vertex_shader_src_len = 0;

    if (file_read("vertex_shader.glsl", vertex_shader_src,
                  vertex_shader_src_capacity, &vertex_shader_src_len) > 1) {
        fprintf(stderr, "Could not open file `%s`: %s", "vertex_shader.glsl",
                strerror(errno));
        exit(errno);
    }

    const usize fragment_shader_src_capacity = 5000;
    u8* fragment_shader_src = ogl_malloc(fragment_shader_src_capacity);
    usize fragment_shader_src_len = 0;

    if (file_read("fragment_shader.glsl", fragment_shader_src,
                  fragment_shader_src_capacity, &fragment_shader_src_len) > 1) {
        fprintf(stderr, "Could not open file `%s`: %s", "fragment_shader.glsl",
                strerror(errno));
        exit(errno);
    }

    return 0;
}
