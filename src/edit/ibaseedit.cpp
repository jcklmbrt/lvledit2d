#include <array>
#include <cfloat>
#include <numeric>

#include "src/gl/glcanvas.hpp"
#include "src/edit/ibaseedit.hpp"

IBaseEdit::IBaseEdit(GLCanvas *canvas)
	: m_canvas(canvas),
	m_edit(canvas->GetEditor()),
	m_view(canvas->GetView())
{
}


IBaseEdit::~IBaseEdit()
{

}


void IBaseEdit::OnDraw()
{

}

void IBaseEdit::DrawPolygon(const ConvexPolygon *p)
{
	bool is_selected = false;
	ConvexPolygon *selected = m_edit.GetSelectedPoly();

	if(selected == p) {
		Rect2D aabb = p->GetAABB();
		m_canvas->OutlineRect(aabb, 1.0f, BLUE);

		/* inflate aabb to illustrate planes */
		aabb.mins -= 1;
		aabb.maxs += 1;

		std::array aabbpts = {
			glm::i32vec2 { aabb.mins.x, aabb.mins.y },
			glm::i32vec2 { aabb.maxs.x, aabb.mins.y },
			glm::i32vec2 { aabb.maxs.x, aabb.maxs.y },
			glm::i32vec2 { aabb.mins.x, aabb.maxs.y }
		};

		for(const Plane2D &plane : p->GetPlanes()) {
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

			wxASSERT(n == 2);
			m_canvas->DrawLine(line[0], line[1], 1.0, RED);
		}
	}

	const std::vector<glm::vec2> &pts = p->GetPoints();

	m_canvas->OutlinePoly(pts.data(), pts.size(), 3.0, BLACK);

	if(selected == p) {
		m_canvas->OutlinePoly(pts.data(), pts.size(), 1.0, GREEN);
	}
	else {
		m_canvas->OutlinePoly(pts.data(), pts.size(), 1.0, WHITE);
	}

	if(p->GetTexture() != nullptr) {
		m_canvas->TexturePoly(*p, WHITE);
	}
}