#ifndef _BYTE_FLOW_GL_UTILS_H_
#define _BYTE_FLOW_GL_UTILS_H_

#include <GLES3/gl3.h>
#include <string>
#include <sstream>
#include <iomanip>


#define FUN_BEGIN_TIME(FUN) {\
    LOGCATE("%s:%s func start", __FILE__, FUN); \
    long long t0 = GetSysCurrentTime();

#define FUN_END_TIME(FUN) \
    long long t1 = GetSysCurrentTime(); \
    LOGCATE("%s:%s func cost time %ldms", __FILE__, FUN, (long)(t1-t0));}

#define BEGIN_TIME(FUN) {\
    LOGCATE("%s func start", FUN); \
    long long t0 = GetSysCurrentTime();

#define END_TIME(FUN) \
    long long t1 = GetSysCurrentTime(); \
    Opipe::Log("OlaGL", "%s func cost time %ldms", FUN, (long)(t1-t0));}

static long long GetSysCurrentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    long long curTime = ((long long) (time.tv_sec)) * 1000 + time.tv_usec / 1000;
    return curTime;
}

#define LOGCATE(...) Opipe::Log("OlaGL", __VA_ARGS__)



class GLUtils {
public:
    static GLuint LoadShader(GLenum shaderType, const char *pSource);

    static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource,
                                GLuint &vertexShaderHandle,
                                GLuint &fragShaderHandle);

    static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource);

    static std::string & ReplaceAllDistinct(std::string &str, const std::string &old_value, const std::string &new_value) ;

    static GLuint CreateProgramWithFeedback(
            const char *pVertexShaderSource,
            const char *pFragShaderSource,
            GLuint &vertexShaderHandle,
            GLuint &fragShaderHandle,
            const GLchar **varying,
            int varyingCount);

    static void DeleteProgram(GLuint &program);

    static void CheckGLError(const char *pGLOperation);

    static std::string GLErrorString(GLenum error);


};

#endif // _BYTE_FLOW_GL_UTILS_H_