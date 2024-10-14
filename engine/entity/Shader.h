#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Resource.h"

namespace engine::entity {

struct Shader;

/// @todo move to data?
struct Shader : public Resource {
    unsigned vertex, fragment;
    unsigned program;
    int  success;
    char infoLog[512];

    Shader(const char *vertex, const char *fragment) {
        // vertex shader
        this->vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(this->vertex, 1, &vertex, nullptr);
        glCompileShader(this->vertex);
        
        glGetShaderiv(this->vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(this->vertex, 1024, nullptr, infoLog);
            std::cout << "[Error] Failed to compile vertex shader:" << std::endl << infoLog << std::endl;
            return;
        }

        // fragment Shader
        this->fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(this->fragment, 1, &fragment, nullptr);
        glCompileShader(this->fragment);
        
        glGetShaderiv(this->fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(this->fragment, 1024, nullptr, infoLog);
            std::cout << "[Error] Failed to compile fragment shader:" << std::endl << infoLog << std::endl << fragment << std::endl;
            return;
        }

        program = glCreateProgram();
        glAttachShader(program, this->vertex);
        glAttachShader(program, this->fragment);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cout << "[Error] Failed to compile program:" << std::endl << infoLog << std::endl;
        }

        glDeleteShader(this->vertex);
        glDeleteShader(this->fragment);
    }

    void use() {
        glUseProgram(program); 
    }

    void setBool(const std::string &name, bool value) const {         
        glUniform1i(glGetUniformLocation(program, name.c_str()), (int)value); 
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(program, name.c_str()), value); 
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(program, name.c_str()), value); 
    }

    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(program, name.c_str()), x, y); 
    }

    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z); 
    }

    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(program, name.c_str()), x, y, z, w); 
    }

    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setTex(const std::string &name, unsigned tex, unsigned idx) {
        glUniform1i(glGetUniformLocation(program, name.c_str()), idx); 
        glActiveTexture(GL_TEXTURE0 + idx);
        glBindTexture(GL_TEXTURE_2D, tex);
    }

    /// @note for opengl 4.2+
    // void setImage(const std::string &name, unsigned im, unsigned idx) {
    //     glUniform1i(glGetUniformLocation(program, name.c_str()), idx); 
    //     // glActiveTexture(GL_TEXTURE0 + idx);
    //     glBindImageTexture(idx, im, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8UI);
    // }

    void setArray(const std::string &name, int count, unsigned *value) {
        glUniform1uiv(glGetUniformLocation(program, name.c_str()), count, value);
    }

    void setFloatArray(const std::string &name, int count, float *value) {
        glUniform1fv(glGetUniformLocation(program, name.c_str()), count, value);
    }
};

} // namespace engine::data

#endif