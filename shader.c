#include "shader.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define BUFFER_CAPACITY 1000
static u8 buffer[BUFFER_CAPACITY] = "";

static void shader_compile(GLuint shader_id, const char path[]) {
    usize shader_src_len = 0;

    if (file_read(path, buffer, BUFFER_CAPACITY, &shader_src_len) != 0) {
        exit(errno);
    }
    nul_terminate(buffer, shader_src_len);
    shader_src_len += 1;

    // Load
    const GLchar* buffer_ptr = (const GLchar*)&buffer;
    glShaderSource(shader_id, 1, &buffer_ptr, NULL);
    // Compile
    glCompileShader(shader_id);

    // Check for errors
    GLint compile_result = false;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, (GLint*)&compile_result);

    i32 compile_info_len = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, (GLint*)&compile_info_len);

    if (compile_info_len > 0) {
        // There was an error, retrieve it
        memset(buffer, 0, BUFFER_CAPACITY);
        assert(compile_info_len + 1 < BUFFER_CAPACITY);

        const usize err_msg_len =
            MIN((usize)compile_info_len + 1, BUFFER_CAPACITY);

        glGetShaderInfoLog(shader_id, compile_info_len, NULL, (GLchar*)buffer);
        fprintf(stderr, "Error compiling the shader: %.*s\n", (int)err_msg_len,
                buffer);
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
    GLint compile_result = false;
    glGetProgramiv(program_id, GL_LINK_STATUS, (GLint*)&compile_result);
    i32 compile_info_len = 0;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &compile_info_len);

    if (compile_info_len > 0) {
        // There was an error, retrieve it
        memset(buffer, 0, BUFFER_CAPACITY);
        assert(compile_info_len + 1 < BUFFER_CAPACITY);

        glGetProgramInfoLog(program_id, compile_info_len, NULL,
                            (GLchar*)buffer);
        fprintf(stderr, "Error linking the shader: %s\n", buffer);
        exit(1);
    }

    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}
