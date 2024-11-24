#ifndef _GLCANVAS_HPP
#define _GLCANVAS_HPP

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>

class GLCanvas : public wxGLCanvas
{
	static constexpr glm::vec2 MAX_PAN = { 10000.0f, 10000.0f };
	static constexpr glm::vec2 MIN_PAN = {-10000.0f,-10000.0f };
	static constexpr float MAX_ZOOM = 10.0;
	static constexpr float MIN_ZOOM = 0.01;
public:
	GLCanvas(wxFrame *parent, const wxGLAttributes &attrs);
private:
	/* Events */
	void OnPaint(wxPaintEvent &e);
	void OnMouse(wxMouseEvent &e);
	void OnScroll(wxScrollEvent &e);
	void OnSize(wxSizeEvent &e);
	void OnKeyDown(wxKeyEvent &e);

	wxFrame     *m_parent  = nullptr;
	wxGLContext *m_context = nullptr;

	glm::vec2 m_pan { 0, 0 };
	float     m_zoom = 1.0;

	void Pan(glm::vec2 delta);
	void Zoom(float factor);
	void ZoomAt(glm::vec2 p, float factor);

	wxDECLARE_EVENT_TABLE();
private:
	void AdjustScale(glm::vec2 mpos, float fraction);
private:
	/* Draw Routines */
	void FillRect(float x, float y, float w, float h, glm::u8vec4 color);
};

#endif