
#include "src/geometry.hpp"
#include <wx/geometry.h>

bool LineLine(double x1, double y1,
              double x2, double y2,
              double x3, double y3,
              double x4, double y4,
              vec2_t *out)
{
	double a = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
	double b = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));

	if(a > 0 && a < 1 && b > 0 && b < 1) {
		if(out != nullptr) {
		 	out->m_x = x1 + (a * (x2-x1));
		 	out->m_y = y1 + (a * (y2-y1));
		}
		return true;
	} else {
		return false;
	}
}

bool LineLine(line_t a, line_t b, vec2_t *out)
{
	return LineLine(a.start.m_x, a.start.m_y,
	                a.end.m_x,   a.end.m_y,
	                b.start.m_x, b.start.m_y,
	                b.end.m_x,   b.end.m_y,
	                out);
}

bool LineRect(line_t a, rect_t b, vec2_t out[2])
{
	
}


bool LineRect(double x0, double y0,
              double x1, double y1,
              rect_t rect, vec2_t out[2])
{
	vec2_t lt = rect.GetLeftTop();
	vec2_t rt = rect.GetRightTop();
	vec2_t lb = rect.GetLeftBottom();
	vec2_t rb = rect.GetRightBottom();

	return  LineLine(lt.m_x, lt.m_y, rt.m_x, rt.m_y, x0, y0, x1, y1) || /* top */
		LineLine(lb.m_x, lb.m_y, rb.m_x, rb.m_y, x0, y0, x1, y1) || /* bottom */
		LineLine(lt.m_x, lt.m_y, lb.m_x, lb.m_y, x0, y0, x1, y1) || /* left */
		LineLine(rt.m_x, rt.m_y, rb.m_x, rb.m_y, x0, y0, x1, y1);   /* right */
}


wxRect RectAroundPoint(double x, double y, double size)
{
	wxRect r;
	double size_2 = size / 2.0;
	r.x = x - size_2;
	r.y = y - size_2;
	r.width  = size;
	r.height = size;
	return r;
}
