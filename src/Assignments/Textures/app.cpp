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
#include "spdlog/spdlog.h"

#define STB_IMAGE_IMPLEMENTATION  1

#include "3rdParty/stb/stb_image.h"

void SimpleShapeApplication::init() {
    xe::ColorMaterial::init();

    stbi_set_flip_vertically_on_load(true);
    GLint width, height, channels;
    auto texture_file = std::string(ROOT_DIR) + "/Models/multicolor.png";
    auto img = stbi_load(texture_file.c_str(), &width, &height, &channels, 0);
    if (!img) {
        spdlog::warn("Could not read image from file `{}'", texture_file);
    }

    GLenum format = GL_RGB;
    if (channels == 4) {
        format = GL_RGBA;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0u);

    auto pyramid = new xe::Mesh;
    auto material = new xe::ColorMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0));
    material->set_texture(texture);

    // A vector containing the x,y,z vertex coordinates for the triangle.
    std::vector<GLfloat> vertices = {
        // bottom
        -0.5, 0.0, -0.5,
        0.191, 0.5,

        0.5, 0.0, -0.5,
        0.5, 0.191,

        -0.5, 0.0, 0.5,
        0.5, 0.809,

        0.5, 0.0, 0.5,
        0.809, 0.5,

        // front
        0.0, 1.0, 0.0,
        0.0, 1.0,

        -0.5, 0.0, 0.5,
        0.191, 0.5,

        0.5, 0.0, 0.5,
        0.5, 0.809,

        // left
        0.0, 1.0, 0.0,
        0.0, 0.0,

        -0.5, 0.0, -0.5,
        0.5, 0.191,

        -0.5, 0.0, 0.5,
        0.191, 0.5,

        // back
        0.0, 1.0, 0.0,
        1.0, 0.0,

        0.5, 0.0, -0.5,
        0.809, 0.5,

        -0.5, 0.0, -0.5,
        0.5, 0.191,

        // right
        0.0, 1.0, 0.0,
        1.0, 1.0,

        0.5, 0.0, 0.5,
        0.5, 0.809,

        0.5, 0.0, -0.5,
        0.809, 0.5,
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
    pyramid->vertex_attrib_pointer(0, 3, GL_FLOAT, 5 * sizeof(GLfloat), 0);
    pyramid->vertex_attrib_pointer(1, 2, GL_FLOAT, 5 * sizeof(GLfloat), 3 * sizeof(GLfloat));

    pyramid->allocate_index_buffer(indices.size() * sizeof(GLushort), GL_STATIC_DRAW);
    pyramid->load_indices(0, indices.size() * sizeof(GLushort), indices.data());

    pyramid->add_submesh(0, 18, material);

    add_submesh(pyramid);

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
