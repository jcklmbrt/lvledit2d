#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>

#include <glm/fwd.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "src/gl/texture.hpp"
#include "src/gl/glcontext.hpp"
#include "src/geometry.hpp"

class wxGLCanvas;
class Notebook;

constexpr glm::vec2 MAX_PAN = { 1000.0f,  1000.0f };
constexpr glm::vec2 MIN_PAN = { -1000.0f, -1000.0f };
constexpr float MAX_ZOOM = 150.0f;
constexpr float MIN_ZOOM = 5.0f;


namespace act {
enum class type : int32_t {
	LINE,
	RECT,
	MOVE,
	SCALE,
	DEL,
	TEXTURE,
	LAYER
};
using rect = irect2d;
using line = iline2d;
using move = glm::i32vec2;
struct scale {
	glm::i32vec2 origin;
	glm::i32vec2 numer;
	glm::i32vec2 denom;
};
struct texture {
	int32_t index;
	int32_t scale;
};
struct index {
	act::type type;
	uint32_t layer;
	uint32_t poly;
	uint32_t index;
};
struct layer {
	glm::vec4 color;
	//uint32_t color;
};
};

namespace l2d {
struct layer {
	layer(const glm::vec4 &color)
		: color(color), polys() {}
	void rmpoly(size_t i)
	{
		std::vector<size_t>::iterator it;
		it = std::find(polys.begin(), polys.end(), i);
		if(it != polys.end()) {
			polys.erase(it);
		}
	}
	glm::vec4 color;
	std::vector<size_t> polys;
};

struct editor {
	// state mgmt
	editor(int width, int height);
	~editor();

	enum state : uint32_t {
		SELECT = 0,
		LINE = 1,
		RECT = 2,
		TEXTURE = 3,
		ENTITY = 4,
		TOOL_MASK = 0x7,
		// misc discriminator
		IN_EDIT = 1 << 3,
		// rect discriminator
		ONE_POINT = 1 << 4,
		// line discriminator (doesn't use IN_EDIT)
		LINE_START_POINT = LINE,
		LINE_END_POINT = LINE | (1 << 3),
		LINE_SLICE = LINE | (1 << 4)
	};

	bool actstr(long i, int col, char buf[256]);

	void enact(size_t i);
	void unact(size_t i);

	act::index &addindex(act::type type, size_t poly, size_t layer, size_t index);
	void addtexture(const char *path);
	void addlayer(const glm::vec4 &color);
	void deletelayer();
	void undo();
	void redo();
	bool save();
	bool save(const char *path);
	bool load(const char *path);
	void resetpoly(size_t i);
	void resetpolys();

	// drawing routines
	void outlinerect(const irect2d &rect, float thickness, const glm::vec4 &color);
	void outlinepoly(const glm::vec2 points[], size_t npoints, float thickness, const glm::vec4 &color);
	void texturepoly(const glm::vec2 pts[], size_t npts, const irect2d &uv, gl::texture &texture, const glm::vec4 &color);
	void drawpoint(const glm::vec2 &point, const glm::vec4 &color);
	void drawline(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color);
	void drawpoly(const poly2d *p);

	// matrices
	void zoom(glm::vec2 origin, float scale);
	glm::vec2 worldtoscreen(glm::vec2 world) const;
	glm::vec2 screentoworld(glm::vec2 screen) const;

	void setupview();
	void setupproj(float width, float height);

	// ui overlay
	void ui_draw();
	bool ui_findtool(state *tool = nullptr);
	bool ui_lmousedown();
	bool ui_mmotion();

	static constexpr int SELECTION_THRESHOLD = 12;

	// events 
	void paint();
	void resize(int width, int height);

	// mouse events
	void mwheel(double xoffs, double yoffs);
	void mmotion(double x, double y);
	void mmousedown();
	void mmouseup();
	void rmousedown();
	void lmousedown();
	void lmouseup();

	// key events
	void key(int key, int scancode, int action, int mods);
private:
	glm::vec2 m_mpos; // last known screen pos of cursor
	glm::vec2 m_wpos; // last known world pos of cursor
	int m_width;
	int m_height;
	float m_zoom = 25.0f;
	glm::vec2 m_pan = { 0.0f, 0.0f };
	glm::mat4 m_view;
	glm::mat4 m_proj;

	std::vector<layer> m_layers;
	std::vector<poly2d> m_polys;
	std::vector<gl::texture> m_textures;

	uint32_t m_selectedpoly = -1;
	uint32_t m_selectedlayer = -1;
	uint32_t m_selectedtexture = -1;

	// actions
	std::vector<act::rect> m_rects;
	std::vector<act::line> m_lines;
	std::vector<act::move> m_moves;
	std::vector<act::scale> m_scales;
	std::vector<act::texture> m_acttextures;
	std::vector<act::layer> m_actlayers;

	// [0..history]    --> history
	// [history..size] --> future
	std::vector<act::index> m_indices;
	uint32_t m_history = 0;

	// current tool
	state m_state;

	// tool vars
	glm::i32vec2 m_start;
	glm::i32vec2 m_end;

	// line edit
	iline2d m_plane;
	std::vector<glm::vec2> m_points;

	// select edit
	int m_outcode;
	glm::i32vec2 m_delta;

	// zoom/pan ctrl
	bool m_panning = false;
	glm::vec2 m_lastmpos;

	// file data
	std::string m_name;
	std::string m_path = {};

	static gl::ctx *s_gl;
};

constexpr editor::state operator&(editor::state a, editor::state b)
{
	uint32_t ai = static_cast<uint32_t>(a);
	uint32_t bi = static_cast<uint32_t>(b);
	return static_cast<editor::state>(ai & bi);
}

constexpr editor::state operator|(editor::state a, editor::state b)
{
	uint32_t ai = static_cast<uint32_t>(a);
	uint32_t bi = static_cast<uint32_t>(b);
	return static_cast<editor::state>(ai | bi);
}

constexpr editor::state operator&=(editor::state &a, editor::state b) 
{
	return a = a & b;
}

constexpr editor::state operator|=(editor::state &a, editor::state b)
{
	return a = a | b;
}

constexpr editor::state operator~(editor::state a)
{
	uint32_t ai = static_cast<uint32_t>(a);
	return static_cast<editor::state>(~ai);
}

};

#endif
