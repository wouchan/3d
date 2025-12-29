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
#include "Engine/Material.h"

void SimpleShapeApplication::init() {
    xe::ColorMaterial::init();

    // A utility function that reads the shader sources, compiles them and creates the program object
    // As everything in OpenGL we reference program by an integer "handle".
    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    auto pyramid = new xe::Mesh;

    auto material_gray = new xe::ColorMaterial(glm::vec4(0.3, 0.3, 0.3, 1.0));
    auto material_red = new xe::ColorMaterial(glm::vec4(1.0, 0.0, 0.0, 1.0));
    auto material_green = new xe::ColorMaterial(glm::vec4(0.0, 1.0, 0.0, 1.0));
    auto material_blue = new xe::ColorMaterial(glm::vec4(0.0, 0.0, 1.0, 1.0));
    auto material_purple = new xe::ColorMaterial(glm::vec4(1.0, 0.0, 1.0, 1.0));

    // A vector containing the x,y,z vertex coordinates for the triangle.
    std::vector<GLfloat> vertices = {
            // bottom
            -0.5, 0.0, -0.5,
            0.5, 0.0, -0.5,
            -0.5, 0.0, 0.5,
            0.5, 0.0, 0.5,

            // front
            0.0, 1.0, 0.0,
            -0.5, 0.0, 0.5,
            0.5, 0.0, 0.5,

            // left
            0.0, 1.0, 0.0,
            -0.5, 0.0, -0.5,
            -0.5, 0.0, 0.5,

            // back
            0.0, 1.0, 0.0,
            0.5, 0.0, -0.5,
            -0.5, 0.0, -0.5,

            // right
            0.0, 1.0, 0.0,
            0.5, 0.0, 0.5,
            0.5, 0.0, -0.5,
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

    pyramid->allocate_vertex_buffer(vertices.size() * sizeof(GLfloat), GL_STATIC_DRAW);
    pyramid->load_vertices(0, vertices.size() * sizeof(GLfloat), vertices.data());
    pyramid->vertex_attrib_pointer(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), 0);
    pyramid->vertex_attrib_pointer(1, 3, GL_FLOAT, 3 * sizeof(GLfloat), 3 * sizeof(GLfloat));

    pyramid->allocate_index_buffer(indices.size() * sizeof(GLushort), GL_STATIC_DRAW);
    pyramid->load_indices(0, indices.size() * sizeof(GLushort), indices.data());

    pyramid->add_submesh(0, 6, material_gray); // bottom
    pyramid->add_submesh(6, 9, material_red); // front
    pyramid->add_submesh(9, 12, material_green); // left
    pyramid->add_submesh(12, 15, material_blue); // back
    pyramid->add_submesh(15, 18, material_purple); // right

    add_submesh(pyramid);

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
    for (auto m: meshes_) {
        m->draw();
    }
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
