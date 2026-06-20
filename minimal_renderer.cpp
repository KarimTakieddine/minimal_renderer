#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"

int main(int argc, char** argv)
{
    if (glfwInit() != GLFW_TRUE)
    {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Minimal Renderer", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();

        return 2;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();

        return 3;
    }

    renderer::Allocator renderAllocator;
    renderer::allocate(&renderAllocator);
    renderer::uploadMeshes(&renderAllocator);
    renderer::initializeGraphicsState();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        renderer::clearFrameBuffer();
        renderer::render(&renderAllocator);

        glfwSwapBuffers(window);
    }

    renderer::freeShaders(&renderAllocator);
    renderer::freeTextures(&renderAllocator);
    renderer::freeVertexArrays(&renderAllocator);
    renderer::freeBuffers(&renderAllocator);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}