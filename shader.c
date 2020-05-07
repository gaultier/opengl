#include "shader.h"

#include <stdio.h>

#include "malloc.h"
#include "types.h"

GLuint shader_load(const char vertex_file_path[],
                   const char fragment_file_path[]) {
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    const usize vertex_shader_src_capacity = 5000;
    u8* vertex_shader_src = ogl_malloc(vertex_shader_src_capacity);

    const usize fragment_shader_src_capacity = 5000;
    u8* fragment_shader_src = ogl_malloc(fragment_shader_src_capacity);

    return 0;
}
