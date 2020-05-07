#include "shader.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "malloc.h"
#include "types.h"

static void shader_compile(GLuint shader_id, const char path[]) {
    const usize shader_src_capacity = 5000;
    u8* const shader_src = ogl_malloc(shader_src_capacity);
    usize shader_src_len = 0;

    if (file_read(path, shader_src, shader_src_capacity, &shader_src_len) !=
        0) {
        exit(errno);
    }

    // Load
    glShaderSource(shader_id, 1, (const GLchar* const*)&shader_src, NULL);
    // Compile
    glCompileShader(shader_id);

    // Check for errors
    bool compile_result = false;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, (GLint*)&compile_result);

    i32 compile_info_len = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, (GLint*)&compile_info_len);

    if (compile_info_len > 0) {
        // There was an error, retrieve it
        const usize err_msg_len = compile_info_len + 1;
        u8* err_msg = ogl_malloc(err_msg_len);
        glGetShaderInfoLog(shader_id, compile_info_len, NULL, (GLchar*)err_msg);
        fprintf(stderr, "Error compiling the shader: `%.*s`\n",
                (int)err_msg_len, err_msg);
        free(err_msg);
        exit(1);
    }
}

GLuint shader_load(const char vertex_file_path[],
                   const char fragment_file_path[]) {
    const GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    shader_compile(vertex_shader_id, vertex_file_path);
    shader_compile(fragment_shader_id, fragment_file_path);

    // Link
    const GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    // Check for link errors
    bool compile_result = false;
    glGetProgramiv(program_id, GL_LINK_STATUS, (GLint*)&compile_result);
    i32 compile_info_len = 0;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &compile_info_len);

    if (compile_info_len > 0) {
        // There was an error, retrieve it
        const usize err_msg_len = compile_info_len + 1;
        u8* err_msg = ogl_malloc(err_msg_len);
        glGetShaderInfoLog(vertex_shader_id, compile_info_len, NULL,
                           (GLchar*)err_msg);
        fprintf(stderr, "Error linking the shader: `%.*s`\n", (int)err_msg_len,
                err_msg);
        free(err_msg);
        exit(1);
    }

    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}
