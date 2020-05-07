#include "shader.h"

#include <errno.h>
#include <stdbool.h>
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
    u8* const vertex_shader_src = ogl_malloc(vertex_shader_src_capacity);
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

    // Load
    glShaderSource(vertex_shader_id, 1, (const GLchar* const)vertex_shader_src,
                   NULL);
    // Compile
    glCompileShader(vertex_shader_id);

    bool compile_result = false;
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, (GLint*)&compile_result);

    i32 compile_info_len = 0;
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH,
                  (GLint*)&compile_info_len);

    if (compile_info_len > 0) {
        // There was an error
        u8* err_msg = ogl_malloc(compile_info_len + 1);
        glGetShaderInfoLog(vertex_shader_id, compile_info_len, NULL,
                           (GLchar*)err_msg);
        nul_terminate(err_msg, compile_info_len + 1);
        printf("Error with shader: `%s`\n", err_msg);
    }

    return 0;
}
