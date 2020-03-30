#include <array>
#include <iterator>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main(int argc, char const *argv[])
{
    glfwInit();

    GLFWwindow * window = glfwCreateWindow( 800, 600, "window", {}, {} );
    glfwMakeContextCurrent( window );
    gladLoadGL();

    while ( !glfwWindowShouldClose(window) )
    {
        void render(); 
        render();

        glfwPollEvents();
        glfwSwapBuffers( window );
    }

    return 0;
}

void render()
{
    std::array<GLfloat,4> color{0.75,0,0,0};
    glClearBufferfv(GL_COLOR, 0, std::data(color));
}