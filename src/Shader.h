#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

// Encapsulates compiling, linking, and binding an OpenGL GLSL shader program.
class Shader {
public:
    GLuint id = 0;

    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    static std::string readFile(const std::string& path);
    static GLuint compileStage(GLenum type, const std::string& source, const std::string& label);
};

