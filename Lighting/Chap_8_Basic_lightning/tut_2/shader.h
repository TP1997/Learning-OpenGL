#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader{
    public:
    unsigned int ID;

    Shader(const GLchar *vertexPath, const GLchar* fragmentPath);
    void use();

    void setBool(const std::string &, bool) const;
    void setInt(const std::string &, int) const;
    void setFloat(const std::string &, float) const;
    void setVec2(const std::string &, glm::vec2 &) const;
    void setVec2(const std::string &, float, float) const;
    void setVec3(const std::string &, glm::vec3 &) const;
    void setVec3(const std::string &, float, float, float) const;
    void setVec4(const std::string &, glm::vec4 &) const;
    void setVec4(const std::string &, float, float, float, float) const;
    void setMat2(const std::string &, glm::mat2 &) const;
    void setMat3(const std::string &, glm::mat3 &) const;
    void setMat4(const std::string &, glm::mat4 &) const;

    void deleteProgram();

    private:
    void checkCompileErrors(unsigned int shader, std::string type);

};

#endif // SHADER_H_INCLUDED
