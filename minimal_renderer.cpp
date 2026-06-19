#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
        glfwTerminate();
        return 3;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // render

        glfwSwapBuffers(window);
    }
    
    glfwTerminate();

    return 0;
}