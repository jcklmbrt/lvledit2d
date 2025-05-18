#include <cstdlib>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>
#include <res/icon_atlas.png.hpp>
namespace ia = icon_atlas;

#include "src/edit/editorcontext.hpp"

static GLFWwindow *s_window = nullptr;
static unsigned char *s_icon = nullptr;
static int s_width = 640;
static int s_height = 480;

size_t m_selectededitor = -1;
std::vector<l2d::editor> s_notebook;


static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	(void)window;

	s_width = width;
	s_height = height;

	if(m_selectededitor != -1) {
		l2d::editor &ed = s_notebook[m_selectededitor];
		ed.resize(width, height);
	}
}

static void cursor_position_callback(GLFWwindow *window, double x, double y)
{
	if(m_selectededitor != -1) {
		l2d::editor &ed = s_notebook[m_selectededitor];
		ed.mmotion(x, y);
	}
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	if(m_selectededitor != -1) {
		l2d::editor &ed = s_notebook[m_selectededitor];
		ed.mwheel(xoffset, yoffset);
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if(m_selectededitor != -1) {
		l2d::editor &ed = s_notebook[m_selectededitor];
		switch(button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			if(action == GLFW_PRESS) {
				ed.rmousedown();
			}
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			if(action == GLFW_PRESS) {
				ed.mmousedown();
			} else if (action == GLFW_RELEASE) {
				ed.mmouseup();
			}
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			if(action == GLFW_PRESS) {
				ed.lmousedown();
			}
			else if(action == GLFW_RELEASE) {
				ed.lmouseup();
			}
			break;
		default:
			break;
		}
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(m_selectededitor != -1) {
		l2d::editor &ed = s_notebook[m_selectededitor];
		ed.key(key, scancode, action, mods);
	}
}

void set_window_icon(unsigned char data[], int width, int height)
{
	GLFWimage icon;
	icon.pixels = data;
	icon.width = width;
	icon.height = height;
	glfwSetWindowIcon(s_window, 1, &icon);
}


int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	if(glfwInit() != GLFW_TRUE) {
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	s_window = glfwCreateWindow(s_width, s_height, "lvledit2d", nullptr, nullptr);
	if(s_window == nullptr) {
		return EXIT_FAILURE;
	}

	int w, h, nchan;
	unsigned char *data;
	data = stbi_load_from_memory(ia::data, sizeof(ia::data), &w, &h, &nchan, 4);
	assert(data);
	assert(nchan == 4);

	GLFWimage icon;
	icon.pixels = new unsigned char[w * h * nchan];
	icon.width = ia::icon.w;
	icon.height = ia::icon.h;

	for(size_t x = 0; x < ia::icon.w; x++) {
		for(size_t y = 0; y < ia::icon.h; y++) {
			size_t dst = (y * ia::icon.w + x) * nchan;
			size_t src = ((y + ia::icon.y) * w + (x + ia::icon.x)) * nchan;
			for(size_t p = 0; p < nchan; p++) {
				icon.pixels[dst + p] = data[src + p];
			}
		}
	}

	glfwGetWindowSize(s_window, &s_width, &s_height);
	framebuffer_size_callback(s_window, s_width, s_height);

	glfwSetFramebufferSizeCallback(s_window, &framebuffer_size_callback);
	glfwSetCursorPosCallback(s_window, &cursor_position_callback);
	glfwSetScrollCallback(s_window, &scroll_callback);
	glfwSetMouseButtonCallback(s_window, &mouse_button_callback);
	glfwSetKeyCallback(s_window, &key_callback);

	glfwMakeContextCurrent(s_window);

	m_selectededitor = 0;
	s_notebook.emplace_back(s_width, s_height);

	while(!glfwWindowShouldClose(s_window)) {
		glfwWaitEvents();
		if(m_selectededitor != -1) {
			l2d::editor &ed = s_notebook[m_selectededitor];
			ed.paint();
			glfwSwapBuffers(s_window);
		}
	}

	if(s_window != nullptr) {
		glfwDestroyWindow(s_window);
	}

	if(s_icon != nullptr) {
		delete[] s_icon;
	}

	return EXIT_SUCCESS;
}