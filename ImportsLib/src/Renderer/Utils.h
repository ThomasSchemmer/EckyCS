#pragma once

#ifdef DEBUG
#define GL_CHECK_ERROR() \
while (GLenum error = glGetError()) { \
printf("OpenGL Error: %d in %s at line %d\n", error, __FILE__, __LINE__); \
__debugbreak(); /* Or some other error handling/logging */ \
}
#else
#define GL_CHECK_ERROR()
#endif

