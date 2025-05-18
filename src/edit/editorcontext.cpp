#include <array>
#include <cstdio>
#include <numeric>

#include <glm/gtc/matrix_transform.hpp>

#include "src/geometry.hpp"
#include "src/gl/glcontext.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/gl/texture.hpp"
#include "src/gl/glcontext.hpp"

#include "res/icon_atlas.png.hpp"
namespace ia = icon_atlas;

gl::ctx *l2d::editor::s_gl = nullptr;

static void ocpts(const irect2d &r, int oc, glm::i32vec2 &corner, glm::i32vec2 &opposite)
{
	corner = r.mins + (r.size() / 2);
	opposite = corner;

	if(oc & irect2d::LEFT) {
		corner.x = r.maxs.x;
		opposite.x = r.mins.x;
	} else if(oc & irect2d::RIGHT) {
		corner.x = r.mins.x;
		opposite.x = r.maxs.x;
	}

	if(oc & irect2d::TOP) {
		corner.y = r.maxs.y;
		opposite.y = r.mins.y;
	} else if(oc & irect2d::BOTTOM) {
		corner.y = r.mins.y;
		opposite.y = r.maxs.y;
	}
}


bool l2d::editor::load(const char *path)
{
	m_name = path;
	m_path = path;

	l2d::file file;

	if(!file.load(m_path.c_str())) {
		return false;
	}

	if(!file.save(*this)) {
		return false;
	}

	resetpolys();

	return true;
}


bool l2d::editor::save()
{
	if(m_path.empty()) {
		return false;
	}

	l2d::file l2file;
	l2file.load(*this);
	l2file.save(m_path.c_str());

	return true;
}


bool l2d::editor::save(const char *path)
{
	//wxASSERT(filename.GetExt() == "l2d");

	m_name = path;
	m_path = path;

	return save();
}


l2d::editor::editor(int width, int height)
	: m_name("untitled"),
	  m_width(width),
	  m_height(height)
{
	if(s_gl == nullptr) {
		s_gl = new gl::ctx();
	}

	// projection matrix
	float fwidth = static_cast<float>(width);
	float fheight = static_cast<float>(height);
	m_proj = glm::ortho(0.0f, fwidth, fheight, 0.0f);

	setupproj(fwidth, fheight);
	setupview();

	// default layer
	m_layers.emplace_back(RED);
	m_selectedlayer = m_layers.size() - 1;

	// select current tool
	m_state = state::SELECT;
}


l2d::editor::~editor()
{
	if(m_path.empty()) {
		save();
	}
}


void l2d::editor::setupview()
{
	glm::vec3 zoom = { m_zoom, m_zoom, 1.0 };
	glm::vec3 pan = { m_pan, 0.0 };

	m_view = glm::identity<glm::mat4>();
	m_view = glm::scale(m_view, zoom);
	m_view = glm::translate(m_view, pan);
}


void l2d::editor::setupproj(float width, float height)
{
	m_proj = glm::ortho(0.0f, width, height, 0.0f);
}


void l2d::editor::zoom(glm::vec2 origin, float factor)
{
	float zoom = m_zoom * factor;
	if(zoom > MAX_ZOOM || zoom < MIN_ZOOM) {
		return;
	}

	glm::vec2 pan = (m_pan - origin / m_zoom) + origin / zoom;

	if(pan.x > MAX_PAN.x || pan.x < MIN_PAN.x) {
		return;
	}

	if(pan.y > MAX_PAN.y || pan.y < MIN_PAN.y) {
		return;
	}

	m_zoom = zoom;
	m_pan = pan;

	setupview();
}


glm::vec2 l2d::editor::worldtoscreen(glm::vec2 world) const
{
	return m_view * glm::vec4(world, 0.0, 1.0);
}


glm::vec2 l2d::editor::screentoworld(glm::vec2 screen) const
{
	return inverse(m_view) * glm::vec4(screen, 0.0, 1.0);
}


void l2d::editor::drawpoly(const poly2d *p)
{
	bool is_selected = false;
	poly2d *selected = nullptr;
	if(m_selectedpoly != -1) {
		selected = &m_polys[m_selectedpoly];
	}

	if(selected == p) {
		irect2d aabb = p->aabb();
		outlinerect(aabb, 1.0f, BLUE);

		/* inflate aabb to illustrate planes */
		aabb.mins -= 1;
		aabb.maxs += 1;

		std::array aabbpts = {
			glm::i32vec2 { aabb.mins.x, aabb.mins.y },
			glm::i32vec2 { aabb.maxs.x, aabb.mins.y },
			glm::i32vec2 { aabb.maxs.x, aabb.maxs.y },
			glm::i32vec2 { aabb.mins.x, aabb.maxs.y }
		};

		for(const iline2d &plane : p->planes()) {
			glm::vec2 line[2]{};
			size_t n = 0;

			for(size_t i = 0; i < aabbpts.size(); i++) {
				glm::i64vec2 p1 = aabbpts[i];
				glm::i64vec2 p2 = aabbpts[(i + 1) % aabbpts.size()];
				glm::i64vec2 delta = p2 - p1;

				int64_t a = plane.a;
				int64_t b = plane.b;
				int64_t c = plane.c;

				int64_t denom = a * delta.x + b * delta.y;
				int64_t numer = -(a * p1.x + b * p1.y + c);

				if(denom != 0) {
					int64_t g = std::gcd(denom, numer);
					if(g != 0) {
						denom /= g;
						numer /= g;
					}

					float t = static_cast<float>(numer) / static_cast<float>(denom);
					if(t >= 0.0f && t <= 1.0f) {
						line[n++] = glm::vec2(p1) + t * glm::vec2(delta);
					}
				}
				else if(numer == 0) {
					line[0] = p1;
					line[1] = p2;
					n = 2;
					break;
				}
				if(n >= 2) {
					break;
				}
			}

			assert(n == 2);
			drawline(line[0], line[1], 1.0, RED);
		}
	}

	const std::vector<glm::vec2> &pts = p->points();

	outlinepoly(pts.data(), pts.size(), 3.0, BLACK);

	if(selected == p) {
		outlinepoly(pts.data(), pts.size(), 1.0, GREEN);
	} else {
		outlinepoly(pts.data(), pts.size(), 1.0, WHITE);
	}

	if(p->texindex < m_textures.size() && p->texindex != -1) {
		texturepoly(pts.data(), pts.size(), p->uv(), m_textures[p->texindex], WHITE);
	}

	if((m_state & state::TOOL_MASK) == state::SELECT && p == selected) {
		const irect2d &aabb = p->aabb();

		std::array aabbpts = {
			glm::vec2 { aabb.mins.x, aabb.mins.y },
			glm::vec2 { aabb.maxs.x, aabb.mins.y },
			glm::vec2 { aabb.mins.x, aabb.maxs.y },
			glm::vec2 { aabb.maxs.x, aabb.maxs.y }
		};

		int oc;
		bool highlight = true;
		glm::vec4 color = PINK;

		if(static_cast<int>(m_state & state::IN_EDIT)) {
			oc = m_outcode;
			color = RED;
		} else {
			glm::vec2 wpos = m_wpos;
			oc = aabb.outcode(wpos);

			glm::vec2 mins = worldtoscreen(aabb.mins);
			glm::vec2 maxs = worldtoscreen(aabb.maxs);
			glm::vec2 mpos = worldtoscreen(wpos);

			mins -= SELECTION_THRESHOLD;
			maxs += SELECTION_THRESHOLD;

			if(mpos.x > maxs.x || mpos.y > maxs.y ||
			   mpos.x < mins.x || mpos.y < mins.y) {
				highlight = false;
			}
		}

		int out_x = oc & irect2d::OUTX;
		int out_y = oc & irect2d::OUTY;

		glm::i32vec2 corner, opposite;
		ocpts(aabb, oc, corner, opposite);

		if(highlight) {
			if(out_x && !out_y) {
				glm::vec2 a = { opposite.x, aabb.mins.y };
				glm::vec2 b = { opposite.x, aabb.maxs.y };
				drawline(a, b, 1.0f, color);
			} else if(out_y && !out_x) {
				glm::vec2 a = { aabb.mins.x, opposite.y };
				glm::vec2 b = { aabb.maxs.x, opposite.y };
				drawline(a, b, 1.0f, color);
			}
		}

		glm::vec2 center = aabb.mins;
		center += glm::vec2(aabb.size()) / 2.0f;
		if(oc == irect2d::INSIDE && highlight) {
			drawpoint(center, color);
		} else {
			drawpoint(center, YELLOW);
		}

		for(glm::vec2 pt : aabbpts) {
			drawpoint(pt, WHITE);
		}

		if(out_x && out_y && highlight) {
			drawpoint(opposite, color);
		}
	}
}


void l2d::editor::mmotion(double x, double y)
{
	m_mpos = { x, y };
	m_wpos = screentoworld(m_mpos);

	ui_mmotion();

	if(m_panning) {
		glm::vec2 delta = m_mpos - m_lastmpos;
		glm::vec2 pan = m_pan + delta / m_zoom;
		if(pan.x > MAX_PAN.x || pan.x < MIN_PAN.x) {
			return;
		}

		if(pan.y > MAX_PAN.y || pan.y < MIN_PAN.y) {
			return;
		}

		m_pan = pan;

		setupview();

		m_lastmpos = m_mpos;
	}

	glm::i32vec2 gpos = s_gl->snaptogrid(m_wpos);

	switch(m_state) {
	// visually update on mouse movement 
	case state::RECT:
		m_start = gpos;
		break;

	case state::RECT | state::IN_EDIT:
	case state::RECT | state::IN_EDIT | state::ONE_POINT:
		m_end = gpos;
		if(m_end.x != m_start.x && m_end.y != m_start.y) {
			m_state &= ~state::ONE_POINT;
		} else {
			m_state |= state::ONE_POINT;
		}
		break;

	case state::SELECT | state::IN_EDIT:
		if(m_selectedlayer == -1 || m_selectedpoly == -1) {
			m_state &= ~state::IN_EDIT;
			break;
		}

		poly2d &selected = m_polys[m_selectedpoly];

		glm::i32vec2 delta = gpos - m_start;
		if(delta.x == 0 && delta.y == 0) {
			return;
		}

		if(m_outcode == irect2d::INSIDE) {

		
			m_start = gpos;

			selected.offset(delta);

			bool intersects = false;
			for(size_t i : m_layers[m_selectedlayer].polys) {
				const poly2d &poly = m_polys[i];
				if(i != m_selectedpoly && selected.intersects(poly)) {
					intersects = true;
					break;
				}
			}

			if(!intersects) {
				// history should never be an invalid value here as we
				// need to at least create a polygon before we move it.
				act::index &back = m_indices[m_history - 1];

				if(back.type == act::type::MOVE && back.poly == m_selectedpoly && back.layer == m_selectedlayer) {
					/* we don't want to spam a move action for each pixel moved */
					m_moves[back.index] += delta;
				} else {
					addindex(act::type::MOVE, m_selectedpoly, m_selectedlayer, m_moves.size());
					m_moves.push_back(delta);
				}
				save();
			} else {
				// go back
				selected.offset(-delta);
			}
		} else {
			irect2d aabb = selected.aabb();
			int outcode = aabb.outcode(m_wpos);

			if(((outcode ^ m_outcode) & irect2d::OUTX) == irect2d::OUTX) {
				m_delta.x = delta.x = 1;
			}

			if(((outcode ^ m_outcode) & irect2d::OUTY) == irect2d::OUTY) {
				m_delta.y = delta.y = 1;
			}

			bool out_x = m_outcode & irect2d::OUTX;
			bool out_y = m_outcode & irect2d::OUTY;
			if(out_x && !out_y) m_delta.y = delta.y = 1;
			if(out_y && !out_x) m_delta.x = delta.x = 1;

			if(m_delta.x == 0 || m_delta.y == 0) {
				m_state &= ~state::IN_EDIT;
				return;
			}

			if(delta.x == 0 || delta.y == 0 || delta == m_delta) {
				return;
			}

			glm::i32vec2 numer = glm::abs(delta);
			glm::i32vec2 denom = glm::abs(m_delta);

			// normalize fraction
			glm::i32vec2 g;
			g.x = std::gcd(numer.x, denom.x);
			g.y = std::gcd(numer.y, denom.y);
			numer /= g;
			denom /= g;

			// make sure scaling will result in a whole number.
			glm::i32vec2 maxs = ((aabb.maxs - m_start) * numer);
			glm::i32vec2 mins = ((aabb.mins - m_start) * numer);
			if(maxs.x % denom.x != 0 || maxs.y % denom.y != 0) {
				return;
			}
			if(mins.x % denom.x != 0 || mins.y % denom.y != 0) {
				return;
			}

			selected.scale(m_start, numer, denom);

			bool intersects = false;
			for(size_t i : m_layers[m_selectedlayer].polys) {
				const poly2d &poly = m_polys[i];
				if(i != m_selectedpoly && selected.intersects(poly)) {
					intersects = true;
					break;
				}
			}

			if(!intersects && denom != numer) {
				m_delta = delta;
				if(m_history != 0 && m_indices.size() != 0) {
					act::index &back = m_indices[m_history - 1];
					if(back.type == act::type::SCALE && back.poly == m_selectedpoly && back.layer == m_selectedlayer) {
						act::scale &back_scale = m_scales[back.index];
						if(m_start == back_scale.origin) {

							back_scale.denom *= denom;
							back_scale.numer *= numer;

							glm::i32vec2 g;
							g.x = std::gcd(back_scale.numer.x, back_scale.denom.x);
							g.y = std::gcd(back_scale.numer.y, back_scale.denom.y);
							back_scale.numer /= g;
							back_scale.denom /= g;

							/* remove if no-op */
							if(back_scale.numer == back_scale.denom) {
								m_history--;
								m_indices.pop_back();
							}
							save();
							return;
						}
					}
				}

				act::index &back = addindex(act::type::SCALE, m_selectedpoly, m_selectedlayer, m_scales.size());

				act::scale scale;
				scale.origin = m_start;
				scale.denom = denom;
				scale.numer = numer;
				m_scales.push_back(scale);
				save();
			} else {
				// go back
				selected.scale(m_start, denom, numer);
			}
		}
		break;
	}
}

void l2d::editor::mwheel(double xoffs, double yoffs)
{
	if(yoffs == 0) {
		/* no scroll */
	} else if(yoffs > 0) { /* scroll up */
		zoom(m_mpos, 1.1f);
	} else { /* scroll down */
		zoom(m_mpos, 0.9f);
	}
}


void l2d::editor::mmousedown()
{
	m_lastmpos = m_mpos;
	m_panning = true;
}


void l2d::editor::mmouseup()
{
	m_panning = false;
}


void l2d::editor::paint()
{
	const glm::vec4 bg = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

	s_gl->clear(bg);
	s_gl->setmatrices(m_proj, m_view);
	s_gl->drawgrid();

	for(layer &layer : m_layers) {
		for(size_t i : layer.polys) {
			if(i != m_selectedpoly) {
				drawpoly(&m_polys[i]);
			}
		}
	}

	poly2d *poly = nullptr;
	if(m_selectedpoly != -1) {
		poly = &m_polys[m_selectedpoly];
		if(m_state != state::LINE_SLICE) {
			drawpoly(poly);
		}
	}

	glm::vec4 color = WHITE;
	glm::vec2 mpos = s_gl->snaptogrid(m_wpos);

	switch(m_state) {
	case state::RECT | state::IN_EDIT:
		if(m_selectedlayer == -1) {
			break;
		}

		irect2d r = irect2d(m_start, m_end);

		for(size_t i : m_layers[m_selectedlayer].polys) {
			poly2d &poly = m_polys[i];
			if(poly.intersects(r)) {
				color = RED;
			}
		}
		// actually draw the rectangle
		outlinerect(r, 3.0, BLACK);
		outlinerect(r, 1.0, color);
		drawpoint(m_start, color);
		drawpoint(m_end, color);
		break;
	case state::RECT | state::IN_EDIT | state::ONE_POINT:
		color = RED;
		[[fallthrough]];
	case state::RECT:
		if(m_selectedlayer == -1) {
			break;
		}
		for(size_t i : m_layers[m_selectedlayer].polys) {
			poly2d &poly = m_polys[i];
			if(poly.contains(m_start)) {
				color = RED;
				break;
			}
		}
		drawpoint(m_start, color);
		break;
	case state::LINE_END_POINT:
		drawline(m_start, mpos, 3.0, BLACK);
		drawline(m_start, mpos, 1.0, RED);
		drawpoint(m_start, WHITE);
		[[fallthrough]];
	case state::LINE_START_POINT:
		drawpoint(mpos, WHITE);
		break;

	case state::LINE_SLICE:
		assert(poly);
		assert(m_selectedlayer != -1);

		outlinepoly(poly->points().data(), poly->points().size(), 3.0, BLACK);
		outlinepoly(poly->points().data(), poly->points().size(), 1.0, RED);

		outlinepoly(m_points.data(), m_points.size(), 3.0, BLACK);
		outlinepoly(m_points.data(), m_points.size(), 1.0, GREEN);

		if(poly->texindex < m_textures.size()) {
			irect2d r;
			r.fit(m_points.data(), m_points.size());
			if(poly->texscale != 0) {
				float scale = static_cast<float>(poly->texscale * gl::ctx::GRID_SPACING);
				glm::vec2 mins = { 0.0f, 0.0f };
				glm::vec2 maxs = { scale, scale };
				r.mins = { 0.0f, 0.0f };
				r.maxs = { scale, scale };
			}
			texturepoly(m_points.data(), m_points.size(), r, m_textures[poly->texindex], WHITE);
		}
		break;
	}

	ui_draw();
}


void l2d::editor::resize(int w, int h)
{
	glViewport(0, 0, w, h);

	m_width = w;
	m_height = h;

	float width = static_cast<float>(w);
	float height = static_cast<float>(h);
	setupproj(width, height);
}


void l2d::editor::outlinerect(const irect2d &rect, float thickness, const glm::vec4 &color)
{
	glm::vec2 lt = rect.mins;
	glm::vec2 rb = rect.maxs;
	glm::vec2 lb = { rect.mins.x, rect.maxs.y };
	glm::vec2 rt = { rect.maxs.x, rect.mins.y };

	thickness /= m_zoom;
	s_gl->line(lt, rt, thickness, color);
	s_gl->line(rt, rb, thickness, color);
	s_gl->line(rb, lb, thickness, color);
	s_gl->line(lb, lt, thickness, color);
}

void l2d::editor::drawline(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color)
{
	thickness /= m_zoom;
	s_gl->line(a, b, thickness, color);
}

void l2d::editor::texturepoly(const glm::vec2 pts[], size_t npts, const irect2d &uv, gl::texture &texture, const glm::vec4 &color)
{
	s_gl->poly(pts, npts, uv, texture, color);
}


void l2d::editor::drawpoint(const glm::vec2 &pt, const glm::vec4 &color)
{
	float thickness = 1.0 / m_zoom;
	glm::vec2 mins = { pt.x - thickness * 3.0f, pt.y - thickness * 3.0f };
	glm::vec2 maxs = { pt.x + thickness * 3.0f, pt.y + thickness * 3.0f };

	s_gl->rect(mins, maxs, BLACK); // outline
	mins += thickness; maxs -= thickness;
	s_gl->rect(mins, maxs, color); // foreground
	mins += thickness; maxs -= thickness;
	s_gl->rect(mins, maxs, BLACK); // center
}


void l2d::editor::outlinepoly(const glm::vec2 points[], size_t npoints, float thickness, const glm::vec4 &color)
{
	thickness /= m_zoom;

	for(size_t i = 0; i < npoints; i++) {
		glm::vec2 a = points[i];
		glm::vec2 b = points[(i + 1) % npoints];
		s_gl->line(a, b, thickness, color);
	}
}

void l2d::editor::resetpolys()
{
	for(layer &layer : m_layers) {
		layer.polys.clear();
	}

	for(size_t i = 0; i < m_history; i++) {
		enact(i);
	}
}


void l2d::editor::resetpoly(size_t i) 
{
	for(size_t a = 0; a < m_indices.size(); a++) {
		if(m_indices[a].poly == i) {
			enact(a);
		}
	}
}


void l2d::editor::undo()
{
	if(m_history <= 0 || m_history > m_indices.size()) {
		return;
	}

	unact(--m_history);
}

void l2d::editor::enact(size_t i)
{
	act::index &act = m_indices[i];

	switch(act.type) {
	case act::type::LINE:
		m_polys[act.poly].slice(m_lines[act.index]);
		m_polys[act.poly].fitlines();
		m_polys[act.poly].fitaabb();
		break;
	case act::type::MOVE:
		m_polys[act.poly].offset(m_moves[act.index]);
		break;
	case act::type::SCALE:
		m_polys[act.poly].scale(m_scales[act.index].origin,
		                        m_scales[act.index].numer,
		                        m_scales[act.index].denom);
		break;
	case act::type::RECT:
		if(act.poly == -1) {
			// initial action
			act.poly = m_polys.size();
			m_polys.emplace_back(m_rects[act.index]);
		}
		else {
			// redo action
			m_polys[act.poly] = m_rects[act.index];
		}
		m_layers[act.layer].polys.push_back(act.poly);
		break;
	case act::type::DEL:
		if(act.poly == -1) {
			m_layers.erase(m_layers.begin() + act.layer);
		} else if(act.layer != -1) {
			m_layers[act.layer].rmpoly(act.poly);
		}
		break;
	case act::type::TEXTURE:
		m_polys[act.poly].texindex = m_acttextures[act.index].index;
		m_polys[act.poly].texscale = m_acttextures[act.index].scale;
		break;
	case act::type::LAYER:
		act.layer = m_layers.size();
		m_layers.emplace_back(m_actlayers[act.index].color);
		break;
	}

	m_selectedpoly = act.poly;

	save();
}


void l2d::editor::deletelayer()
{
	if(!m_layers.empty() && m_selectedlayer != -1) {
		addindex(act::type::DEL, -1, m_selectedlayer, -1);
		enact(m_indices.size() - 1);
		m_selectedpoly  = -1;
		m_selectedlayer = -1;
	}
}


void l2d::editor::unact(size_t i)
{
	act::index &act = m_indices[i];

	switch(act.type) {
	case act::type::TEXTURE:
	case act::type::LINE:
		resetpoly(act.poly);
		break;
	case act::type::MOVE:
		m_polys[act.poly].offset(-m_moves[act.index]);
		break;
	case act::type::SCALE:
		m_polys[act.poly].scale(m_scales[act.index].origin,
		                        m_scales[act.index].denom, 
		                        m_scales[act.index].numer);
		break;
	case act::type::RECT:
		m_selectedpoly = -1;
		assert(act.poly != -1);
		m_layers[act.layer].rmpoly(act.poly);
		break;
	case act::type::LAYER:
		m_selectedlayer = -1;
		m_layers.erase(m_layers.begin() + act.layer);
		break;
	case act::type::DEL:
		m_layers[act.layer].polys.push_back(act.poly);
		break;
	}

	save();
}


void l2d::editor::addtexture(const char *path)
{
	gl::texture texture;
	if(texture.load(path) == false) {
		assert(false);
		return;
	}

	for(gl::texture &t : m_textures) {
		if(texture == t) {
			return;
		}
	}

	m_textures.push_back(texture);
	m_selectedtexture = m_textures.size() - 1;
}


void l2d::editor::redo()
{
	if(m_history < m_indices.size()) {
		enact(m_history++);
	}
}



act::index &l2d::editor::addindex(act::type type, size_t poly, size_t layer, size_t index)
{
	// future will now be invalid
	while(m_indices.size() > m_history) {
		m_indices.pop_back();
	}

	act::index &back = m_indices.emplace_back();
	back.type = type;
	back.poly = poly;
	back.layer = layer;
	back.index = index;

	m_history++;

	return back;
}

void l2d::editor::addlayer(const glm::vec4 &color)
{
	m_selectedpoly = -1;
	m_selectedlayer = m_layers.size();

	act::layer layer;
	layer.color = color;

	act::index &back = addindex(act::type::LAYER, -1, -1, m_actlayers.size());
	m_actlayers.push_back(layer);

	enact(m_indices.size() - 1);
}


void l2d::editor::lmouseup()
{
	switch(m_state) {
	case state::SELECT | state::IN_EDIT:
		m_state &= ~state::IN_EDIT;
		break;
	}
}


bool l2d::editor::actstr(long i, int col, char buf[256])
{
	glm::i32vec2 lt, rb;

	act::index &index = m_indices[i];
	if(col == 0) {
		switch(index.type) {
		case act::type::LINE: strcpy(buf, "LINE"); break;
		case act::type::RECT: strcpy(buf, "RECT"); break;
		case act::type::MOVE: strcpy(buf, "MOVE"); break;
		case act::type::SCALE: strcpy(buf, "SCALE"); break;
		case act::type::TEXTURE: strcpy(buf, "TEXTURE"); break;
		case act::type::DEL: strcpy(buf, "DEL"); break;
		case act::type::LAYER: strcpy(buf, "LAYER"); break;
		}
	} else if(col == 1) {
		switch(index.type) {
		case act::type::LINE:
			snprintf(buf, sizeof(buf), "%dx + %dy + %d", 
			         m_lines[index.index].a, 
			         m_lines[index.index].b,
			         m_lines[index.index].c);
			break;
		case act::type::RECT:
			lt = m_rects[index.index].mins;
			rb = m_rects[index.index].maxs;
			snprintf(buf, sizeof(buf), "%d %d %d %d", 
			         lt.x, lt.y, lt.x + rb.x, lt.y + rb.y);
			break;
		case act::type::MOVE:
			snprintf(buf, sizeof(buf), "%d %d", 
			         m_moves[index.index].x, 
			         m_moves[index.index].y);
			break;
		case act::type::SCALE:
			snprintf(buf, sizeof(buf), "%d/%d %d/%d",
				m_scales[index.index].numer.x, 
				m_scales[index.index].denom.x,
				m_scales[index.index].numer.y, 
				m_scales[index.index].denom.y);
			break;
		case act::type::TEXTURE:
			snprintf(buf, sizeof(buf), "%d x %d",
			         m_acttextures[index.index].index,
			         m_acttextures[index.index].scale);
			break;
		case act::type::LAYER:
		case act::type::DEL:
			break;
		}
	} else if(col == 2) {
		if(index.type != act::type::LAYER && index.poly != -1) {
			snprintf(buf, sizeof(buf), "%u", index.poly);
		} else {
			buf[0] = '\0';
			return false;
		}
	} else if(col == 3) {
		snprintf(buf, sizeof(buf), "%u", index.layer);
	}

	return true;
}



void l2d::editor::lmousedown()
{
	if(ui_lmousedown()) {
		return;
	}

	poly2d *selected = nullptr;
	bool intersects = false;

	if(m_selectedlayer == -1) {
		return;
	}

	layer &layer = m_layers[m_selectedlayer];

	switch(m_state) {
	case state::TEXTURE:
		for(size_t i : layer.polys) {
			if(m_polys[i].contains(m_wpos)) {
				m_selectedpoly = i;
				break;
			}
		}
		if(m_selectedpoly != -1 && m_selectedtexture != -1) {
			act::texture act;
			act.index = m_selectedtexture;
			act.scale = 1;

			if(!m_indices.empty() && m_history != 0) {
				act::index &back = m_indices[m_history - 1];
				/* don't bother saving repeat texture actions */
				if(back.type == act::type::TEXTURE && back.poly == m_selectedpoly) {
					act::texture &back_texture = m_acttextures[back.index];
					if(act.index == back_texture.index && act.scale == back_texture.scale) {
						return;
					}
				}
			}

			act::index &back = addindex(act::type::TEXTURE, m_selectedpoly, m_selectedlayer, m_acttextures.size());
			m_acttextures.push_back(act);

			enact(m_indices.size() - 1);
			save();
		}
		break;
	case state::SELECT:
	case state::SELECT | state::IN_EDIT:
		for(size_t i : layer.polys) {
			if(m_polys[i].contains(m_wpos)) {
				m_selectedpoly = i;
			}
		}
		if(m_selectedpoly != -1) {
			selected = &m_polys[m_selectedpoly];
			const irect2d &aabb = selected->aabb();
			m_state |= state::IN_EDIT;
			m_outcode = aabb.outcode(m_wpos);
			m_start = s_gl->snaptogrid(m_wpos);
			glm::i32vec2 opposite;

			if(m_outcode != irect2d::INSIDE) {
				/* too far away */
				glm::vec2 mins = worldtoscreen(aabb.mins);
				glm::vec2 maxs = worldtoscreen(aabb.maxs);
				glm::vec2 mpos = worldtoscreen(m_wpos);

				mins -= SELECTION_THRESHOLD;
				maxs += SELECTION_THRESHOLD;

				if(mpos.x > maxs.x || mpos.y > maxs.y ||
				   mpos.x < mins.x || mpos.y < mins.y) {
					m_state &= ~state::IN_EDIT;
				}

				ocpts(aabb, m_outcode, m_start, opposite);
				m_delta = opposite - m_start;
			} else {
				m_start = m_wpos;
				m_state |= state::IN_EDIT;
				m_outcode = irect2d::INSIDE;
			}
		}
		break;

	case state::RECT: // 1st click
		for(size_t i : m_layers[m_selectedlayer].polys) {
			const poly2d &poly = m_polys[i];
			if(poly.contains(m_wpos)) {
				intersects = true;
				break;
			}
		}
		if(!intersects) {
			m_end = m_start;
			m_state |= state::IN_EDIT;
			m_state |= state::ONE_POINT;
		}
		break;

	case state::RECT | state::IN_EDIT: // 2nd click
		irect2d r = irect2d(m_start, m_end);
		for(size_t i : m_layers[m_selectedlayer].polys) {
			const poly2d &poly = m_polys[i];
			if(poly.intersects(r)) {
				intersects = true;
				break;
			}
		}
		if(!intersects) {
			act::index &back = addindex(act::type::RECT, -1, m_selectedlayer, m_rects.size());
			m_rects.push_back(r);
			enact(m_indices.size() - 1);
			save();
			// reset flags
			m_state &= ~state::IN_EDIT;
			m_start = s_gl->snaptogrid(m_wpos);
		}
		break;

	case state::LINE_START_POINT:
		if(m_selectedpoly == -1) {
			break;
		}
		m_start = s_gl->snaptogrid(m_wpos);
		m_state = state::LINE_END_POINT;
		break;

	case state::LINE_END_POINT:
		m_end = s_gl->snaptogrid(m_wpos);
		m_plane = iline2d(m_start, m_end);

		if(m_selectedpoly != -1) {
			poly2d &selected = m_polys[m_selectedpoly];
			if(selected.allptsbehind(m_plane)) {
				// bad cut, start over.
				m_state = state::LINE_START_POINT;
			} else {
				m_points.clear();
				selected.addline(m_plane, m_points);
				m_state = state::LINE_SLICE;
			}
		}
		break;

	case state::LINE_SLICE:
		act::index &back = addindex(act::type::LINE, m_selectedpoly, m_selectedlayer, m_lines.size());
		m_lines.push_back(m_plane);
		enact(m_indices.size() - 1);
		save();
		m_state = state::LINE_START_POINT;
		break;
	}
}

void l2d::editor::rmousedown()
{
	poly2d *selected = nullptr;
	if(m_selectedlayer == -1) {
		return;
	}

	layer &layer = m_layers[m_selectedlayer];

	switch(m_state) {
	case state::TEXTURE:
		for(size_t i : layer.polys) {
			if(m_polys[i].contains(m_wpos)) {
				m_selectedpoly = i;
				break;
			}
		}
		if(m_selectedpoly != -1) {
			act::texture act;
			act.index = -1;
			act.scale = 0;

			if(!m_indices.empty() && m_history != 0) {
				act::index &back = m_indices[m_history - 1];
				/* don't bother saving repeat texture actions */
				if(back.type == act::type::TEXTURE) {
					act::texture &back_texture = m_acttextures[back.index];
					if(back.poly == m_selectedpoly
					&& act.index == back_texture.index
					&& act.scale == back_texture.scale) {
						return;
					}
				}
			}

			act::index &back = addindex(act::type::TEXTURE, m_selectedpoly, m_selectedlayer, m_acttextures.size());
			m_acttextures.push_back(act);

			enact(m_indices.size() - 1);
			save();
		}
		break;
	case state::SELECT:
		for(size_t i : layer.polys) {
			if(m_polys[i].contains(m_wpos)) {
				m_selectedpoly = i;
				break;
			}
		}
		break;
	case state::LINE_SLICE:
		m_plane.flip();
		m_points.clear();
		assert(m_selectedlayer != -1);
		assert(m_selectedpoly != -1);
		selected = &m_polys[m_selectedpoly];
		selected->addline(m_plane, m_points);
		break;
	}
}


void l2d::editor::key(int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_DELETE && action == GLFW_PRESS) {
		if(m_selectedpoly != -1 && m_selectedlayer != -1) {
			addindex(act::type::DEL, m_selectedpoly, m_selectedlayer, -1);
			enact(m_indices.size() - 1);
			m_selectedpoly = -1;
		}
	}
}


bool l2d::editor::ui_findtool(state *out)
{
	static constexpr int PAD_X = 4;
	static constexpr int PAD_Y = 4;
	int dy = 0, yoffs = ia::hand.h + PAD_Y;

	for(int i = 0; i < 5; i++) {
		// select tool
		glm::i32vec2 mins = { PAD_X, PAD_Y + dy++ * yoffs };
		glm::i32vec2 maxs = { mins.x + ia::hand.w, mins.y + ia::hand.h };

		irect2d r{ mins, maxs };

		if(r.contains(m_mpos)) {
			if(out) {
				*out = static_cast<state>(i);
			}
			return true;
		}
	}

	return false;
}


bool l2d::editor::ui_mmotion()
{
	if(ui_findtool()) {
		return true;
	}

	return false;
}


bool l2d::editor::ui_lmousedown()
{
	if(ui_findtool(&m_state)) {
		mmotion(m_mpos.x, m_mpos.y);
		return true;
	}

	return false;
}


void l2d::editor::ui_draw()
{
	static glm::mat4 id{ 1 };
	s_gl->setmatrices(m_proj, id);

	glm::vec2 size;
	size.x = static_cast<float>(m_width);
	size.y = static_cast<float>(m_height);

	// draw toolbar
	static constexpr float PAD_X = 4;
	static constexpr float PAD_Y = 4;
	float dy = 0, yoffs = ia::hand.h + PAD_Y;

	float tb_width = ia::hand.h + PAD_X * 2;
	s_gl->rect({ 0, 0 }, { tb_width + 1, size.y }, BLACK);
	s_gl->rect({ 0, 0 }, { tb_width,     size.y }, PASTEL_PINK);

	static constexpr ia::position state_icon[] = {
		ia::hand,
		ia::line,
		ia::rect,
		ia::texture,
		ia::pawn
	};

	for(int i = 0; i < 5; i++) {
		// select tool
		glm::i32vec2 mins = { PAD_X, 
		                      PAD_Y + i * yoffs };

		glm::i32vec2 maxs = { mins.x + state_icon[i].w + 1, 
		                      mins.y + state_icon[i].h + 1 };

		irect2d r = { mins, maxs };

		if(r.contains(m_mpos)) {
			s_gl->rect(mins, maxs, glm::vec4(0.0f, 0.5f, 1.0f, 0.2f));
			outlinerect(r, m_zoom, glm::vec4(0.0f, 0.5f, 1.0f, 0.5f));
			// drop shadow
			s_gl->icon(mins + 1, state_icon[i], glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
		}

		if(i == (m_state & TOOL_MASK)) { // icon is selected
			s_gl->rect(mins, maxs, glm::vec4(0.0f, 0.5f, 1.0f, 0.5f));
			s_gl->icon(mins + 1, state_icon[i], glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
		}

		s_gl->icon(mins, state_icon[i], WHITE);
	}

	constexpr int HLIST_WIDTH = 300;
	constexpr int HLIST_PAD_X = 20;
	constexpr int HLIST_PAD_Y = 10;

	float xofs = size.x - 300;
	s_gl->rect({xofs - 1, 0 }, { size.x, size.y }, BLACK);
	s_gl->rect({xofs, 0}, { size.x, size.y }, PASTEL_PINK);
	
	char buf[256];
	for(size_t i = 0; i < m_indices.size(); i++) {
		dy = m_indices.size() - i;
		actstr(i, 0, buf);
		s_gl->puts({ xofs + HLIST_PAD_X,       HLIST_PAD_Y + dy * 14 }, BLACK, buf);
		actstr(i, 2, buf);
		s_gl->puts({ xofs + HLIST_PAD_X + 60,  HLIST_PAD_Y + dy * 14 }, BLACK, buf);
		actstr(i, 3, buf);
		s_gl->puts({ xofs + HLIST_PAD_X + 100, HLIST_PAD_Y + dy * 14 }, BLACK, buf);
		actstr(i, 1, buf);
		s_gl->puts({ xofs + HLIST_PAD_X + 140, HLIST_PAD_Y + dy * 14 }, BLACK, buf);
	}
}