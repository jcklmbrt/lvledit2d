#ifndef _GLCANVAS_HPP
#define _GLCANVAS_HPP

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>

class GLCanvas : public wxGLCanvas
{
public:
	GLCanvas(wxFrame *parent, const wxGLAttributes &attrs);
private:
	/* Events */
	void OnPaint(wxPaintEvent &e);
	void OnMouse(wxMouseEvent &e);
	void OnSize(wxSizeEvent &e);
	void OnKeyDown(wxKeyEvent &e);

	wxFrame     *m_parent  = nullptr;
	wxGLContext *m_context = nullptr;

	wxDECLARE_EVENT_TABLE();
private:
	/* Draw Routines */
	void FillRect(float x, float y, float w, float h, glm::u8vec4 color);
};

#endif