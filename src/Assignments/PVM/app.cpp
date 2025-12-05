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
            // roof
            -0.5f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            0.5f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            0.0f, 0.5f, 0.0f,
            1.0f, 0.0f, 0.0f,

            // walls
            -0.5f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,

            -0.5f, -0.5f, 0.0f,
            0.0f, 1.0f, 0.0f,

            0.5f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,

            0.5f, -0.5f, 0.0f,
            0.0f, 1.0f, 0.0f,
        };

    std::vector<GLushort> indices = {
        0, 1, 2,
        3, 4, 5,
        5, 4, 6
    };

    float strength = 0.8;
    float color[3] = { 0.5, 0.8, 0.2 };

    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
    glm::mat4 View = glm::lookAt(
        glm::vec3(0.75f, 1.0f, 3.0f), 
  		glm::vec3(0.0f, 0.0f, 0.0f), 
  		glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
    glm::mat4 PVM = Projection * View * Model;

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

    GLuint transform_buffer_handle;
    glGenBuffers(1, &transform_buffer_handle);
    OGL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, transform_buffer_handle));
    glBufferData(GL_UNIFORM_BUFFER, 64, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, transform_buffer_handle);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, &PVM);
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
    glClearColor(0.2f, 0.2f, 1.0f, 1.0f);

    // This setups an OpenGL vieport of the size of the whole rendering window.
    auto[w, h] = frame_buffer_size();
    glViewport(0, 0, w, h);

    glUseProgram(program);
}

//This functions is called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {
    // Binding the VAO will setup all the required vertex buffers.
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}
