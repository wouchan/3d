//
// Created by pbialas on 25.09.2020.
//

#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Application/utils.h"

void SimpleShapeApplication::init() {
    // A utility function that reads the shader sources, compiles them and creates the program object
    // As everything in OpenGL we reference program by an integer "handle".
    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    // A vector containing the x,y,z vertex coordinates for the triangle.
    std::vector<GLfloat> vertices = {
            // bottom
            -0.5, 0.0, -0.5,
            0.3, 0.3, 0.3,

            0.5, 0.0, -0.5,
            0.3, 0.3, 0.3,

            -0.5, 0.0, 0.5,
            0.3, 0.3, 0.3,

            0.5, 0.0, 0.5,
            0.3, 0.3, 0.3,

            // front
            0.0, 1.0, 0.0,
            1.0, 0.0, 0.0,

            -0.5, 0.0, 0.5,
            1.0, 0.0, 0.0,

            0.5, 0.0, 0.5,
            1.0, 0.0, 0.0,

            // left
            0.0, 1.0, 0.0,
            0.0, 1.0, 0.0,

            -0.5, 0.0, -0.5,
            0.0, 1.0, 0.0,

            -0.5, 0.0, 0.5,
            0.0, 1.0, 0.0,

            // back
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0,

            0.5, 0.0, -0.5,
            0.0, 0.0, 1.0,

            -0.5, 0.0, -0.5,
            0.0, 0.0, 1.0,

            // right
            0.0, 1.0, 0.0,
            1.0, 0.0, 1.0,

            0.5, 0.0, 0.5,
            1.0, 0.0, 1.0,

            0.5, 0.0, -0.5,
            1.0, 0.0, 1.0,
        };

    std::vector<GLushort> indices = {
        // bottom
        0, 1, 2,
        2, 1, 3,
        // front
        4, 5, 6,
        // left
        7, 8, 9,
        // back
        10, 11, 12,
        // right
        13, 14, 15,
    };

    float strength = 1.0;
    float color[3] = { 1.0, 1.0, 1.0 };

    int w, h;
    std::tie(w, h) = frame_buffer_size();
    auto aspect = (float)w/h;
    auto fov = glm::pi<float>()/4.0;
    auto near = 0.1f;
    auto far = 100.0f;

    set_camera(new Camera);

    camera()->perspective(fov, aspect, near, far);
    camera()->look_at(
        glm::vec3(2.5f, -0.5f, -3.0f),
  		glm::vec3(0.0f, 0.0f, 0.0f),
  		glm::vec3(0.0f, 1.0f, 0.0f)
    );

    set_controler(new CameraControler(camera()));

    // Generating the buffer and loading the vertex data into it.
    GLuint v_buffer_handle;
    glGenBuffers(1, &v_buffer_handle);
    OGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle));
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint i_buffer_handle;
    glGenBuffers(1, &i_buffer_handle);
    OGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer_handle));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint modifier_buffer_handle;
    glGenBuffers(1, &modifier_buffer_handle);
    OGL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, modifier_buffer_handle));
    glBufferData(GL_UNIFORM_BUFFER, 8 * sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, modifier_buffer_handle);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4, &strength);
    glBufferSubData(GL_UNIFORM_BUFFER, 16, 12, color);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, 64, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // This setups a Vertex Array Object (VAO) that  encapsulates
    // the state of all vertex buffers needed for rendering
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer_handle);

    // This indicates that the data for attribute 0 should be read from a vertex buffer.
    glEnableVertexAttribArray(0);
    // and this specifies how the data is layout in the buffer.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(3 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //end of vao "recording"

    // Setting the background color of the rendering window,
    // I suggest not to use white or black for better debuging.
    glClearColor(0.8f, 0.8f, 1.0f, 1.0f);

    // This setups an OpenGL vieport of the size of the whole rendering window.
    glViewport(0, 0, w, h);

    glUseProgram(program);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

//This functions is called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {
    auto PVM = camera()->projection() * camera()->view();
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Binding the VAO will setup all the required vertex buffers.
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0,0,w,h);
    auto aspect = (float) w / h;
    camera()->set_aspect(aspect);
}

void SimpleShapeApplication::scroll_callback(double xoffset, double yoffset) {
    Application::scroll_callback(xoffset, yoffset);
    camera()->zoom(yoffset / 30.0f);
}

void SimpleShapeApplication::mouse_button_callback(int button, int action, int mods) {
    Application::mouse_button_callback(button, action, mods);

    if (controler_) {
        double x, y;
        glfwGetCursorPos(window_, &x, &y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            controler_->LMB_pressed(x, y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
            controler_->LMB_released(x, y);
    }
}

void SimpleShapeApplication::cursor_position_callback(double x, double y) {
    Application::cursor_position_callback(x, y);
    if (controler_) {
        controler_->mouse_moved(x, y);
    }
}
