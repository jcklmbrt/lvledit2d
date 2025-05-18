#ifndef _GLCONTEXT_HPP
#define _GLCONTEXT_HPP

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stb_truetype.h>

#include "src/geometry.hpp"
#include "src/gl/texture.hpp"

#include "res/icon_atlas.png.hpp"

namespace gl {
struct vtx {
	glm::vec2 pos;
	glm::vec2 uv;
	glm::vec4 color;
};

struct texturebatch {
	std::vector<vtx> vtx;
	std::vector<GLuint> idx;
};

struct ctx {
	ctx();
	~ctx();
	void clear(const glm::vec4 &color);
	void setmatrices(const glm::mat4 &proj, const glm::mat4 &view);
	void drawgrid();
	void begin();
	void end();
	void quad(const glm::vec2 q[4], const glm::vec4 &color);
	void rect(const glm::vec2 &mins, const glm::vec2 &maxs, const glm::vec4 &color);
	void line(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color);
	void poly(const glm::vec2 pts[], size_t npts, const irect2d &uv, gl::texture &texture, const glm::vec4 &color);
	void icon(const glm::vec2 &pos, icon_atlas::position uv, const glm::vec4 &color);
	void puts(const glm::vec2 &pos, const glm::vec4 color, const char *s);
private:
	// gl objects for backgroud grid
	GLuint m_grid_program;
	GLuint m_grid_vtxbuf;
	GLuint m_grid_vao;

	// gl objects for rendering solid geometry
	GLuint m_solid_program;
	std::vector<vtx> m_solid_vtx;
	std::vector<GLuint> m_solid_idx;

	GLuint m_icon_atlas;
	int m_icon_atlas_width;
	int m_icon_atlas_height;

	GLuint m_font_atlas;
	int m_font_atlas_width;
	int m_font_atlas_height;

	// gl object for rendering textured geometry
	GLuint m_texture_program;
	std::unordered_map<GLuint, texturebatch> m_texture_batches;

	GLuint m_vtxbuf;
	GLuint m_idxbuf;
	GLuint m_vao;
public:
	constexpr static int GRID_SPACING = 1;
	[[nodiscard]] static glm::i32vec2 snaptogrid(const glm::vec2 &pt)
	{
		int32_t x = round(pt.x / GRID_SPACING) * GRID_SPACING;
		int32_t y = round(pt.y / GRID_SPACING) * GRID_SPACING;

		return glm::i32vec2(x, y);
	}
};
}

#endif
