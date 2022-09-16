#include "GLUtils.h"
#include <stdlib.h>
#include <cstring>
#include <GLES2/gl2ext.h>
#include "GPUImageMacros.h"

#include "GPUImageUtil.h"

using namespace Opipe;

GLuint GLUtils::LoadShader(GLenum shaderType, const char *pSource)
{
    GLuint shader = 0;
    shader = glCreateShader(shaderType);
    if (shader)
    {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char *buf = (char *)malloc((size_t)infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    Opipe::LogE("GLUtils","GLUtils::LoadShader Could not compile shader %d:\n%s\n%s\n", shaderType, buf, pSource);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

std::string GLUtils::GLErrorString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW_KHR:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW_KHR:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default:
    {
        std::ostringstream oss;
        oss << "<Unknown GL Error 0x" << std::setfill('0') << std::setw(4) << std::right << std::hex << error
            << ">";
        return oss.str();
    }
    }
}

GLuint GLUtils::CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource, GLuint &vertexShaderHandle, GLuint &fragShaderHandle)
{
    GLuint program = 0;
    //    FUN_BEGIN_TIME("GLUtils::CreateProgram")
    vertexShaderHandle = LoadShader(GL_VERTEX_SHADER, pVertexShaderSource);
    if (!vertexShaderHandle)
        return program;
    fragShaderHandle = LoadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
    if (!fragShaderHandle)
        return program;

    program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShaderHandle);
        CheckGLError("glAttachShader");
        glAttachShader(program, fragShaderHandle);
        CheckGLError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

        glDetachShader(program, vertexShaderHandle);
        glDeleteShader(vertexShaderHandle);
        vertexShaderHandle = 0;
        glDetachShader(program, fragShaderHandle);
        glDeleteShader(fragShaderHandle);
        fragShaderHandle = 0;
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char *buf = (char *)malloc((size_t)bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    Opipe::LogE("GLUtils","GLUtils::CreateProgram Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    //    FUN_END_TIME("GLUtils::CreateProgram")
    Opipe::LogE("GLUtils","GLUtils::CreateProgram program = %d", program);
    return program;
}

GLuint GLUtils::CreateProgramWithFeedback(const char *pVertexShaderSource, const char *pFragShaderSource, GLuint &vertexShaderHandle, GLuint &fragShaderHandle, GLchar const **varying, int varyingCount)
{
    GLuint program = 0;
    //    FUN_BEGIN_TIME("GLUtils::CreateProgramWithFeedback")
    vertexShaderHandle = LoadShader(GL_VERTEX_SHADER, pVertexShaderSource);
    if (!vertexShaderHandle)
        return program;

    fragShaderHandle = LoadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
    if (!fragShaderHandle)
        return program;

    program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShaderHandle);
        CheckGLError("glAttachShader");
        glAttachShader(program, fragShaderHandle);
        CheckGLError("glAttachShader");

        // transform feedback
        glTransformFeedbackVaryings(program, varyingCount, varying, GL_INTERLEAVED_ATTRIBS);
        CHECK_GL();

        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

        glDetachShader(program, vertexShaderHandle);
        glDeleteShader(vertexShaderHandle);
        vertexShaderHandle = 0;
        glDetachShader(program, fragShaderHandle);
        glDeleteShader(fragShaderHandle);
        fragShaderHandle = 0;
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char *buf = (char *)malloc((size_t)bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    Opipe::LogE("GLUtils","GLUtils::CreateProgramWithFeedback Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    //    FUN_END_TIME("GLUtils::CreateProgramWithFeedback")
    Opipe::LogE("GLUtils","GLUtils::CreateProgramWithFeedback program = %d", program);
    return program;
}

void GLUtils::DeleteProgram(GLuint &program)
{
    Opipe::LogE("GLUtils","GLUtils::DeleteProgram");
    if (program)
    {
        glUseProgram(0);
        glDeleteProgram(program);
        program = 0;
    }
}

void GLUtils::CheckGLError(const char *pGLOperation)
{
    for (GLint error = glGetError(); error; error = glGetError())
    {
        Opipe::LogE("GLUtils","GLUtils::CheckGLError GL Operation %s() glError (0x%x)\n", pGLOperation, error);
    }
}

GLuint GLUtils::CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource)
{
    GLuint vertexShaderHandle, fragShaderHandle;
    return CreateProgram(pVertexShaderSource, pFragShaderSource, vertexShaderHandle, fragShaderHandle);
}

std::string &GLUtils::ReplaceAllDistinct(std::string &str, const std::string &old_value, const std::string &new_value)
{
    std::string::size_type pos = 0;
    while ((pos = str.find(old_value, pos)) != std::string::npos)
    {
        str = str.replace(pos, old_value.length(), new_value);
        if (new_value.length() > 0)
        {
            pos += new_value.length();
        }
    }
    return str;
}
