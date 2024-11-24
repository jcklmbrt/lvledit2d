#ifndef _GLCANVAS_HPP
#define _GLCANVAS_HPP

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/notebook.h>
#include <glm/glm.hpp>

#include "src/glnotebook.hpp"

class GLCanvas : public wxGLCanvas
{
	static constexpr glm::vec2 MAX_PAN = { 10000.0f, 10000.0f };
	static constexpr glm::vec2 MIN_PAN = {-10000.0f,-10000.0f };
	static constexpr float MAX_ZOOM = 10.0;
	static constexpr float MIN_ZOOM = 0.01;
public:
	GLCanvas(GLNoteBook *parent, const wxGLAttributes &attrs);
private:
	/* Events */
	void OnPaint(wxPaintEvent &e);
	void OnMouse(wxMouseEvent &e);
	void OnSize(wxSizeEvent &e);
	void OnKeyDown(wxKeyEvent &e);

	static glm::mat4 SetupView(glm::vec2 pan, float zoom);

	GLNoteBook *m_parent  = nullptr;

	float m_zoom = 1.0;
	glm::vec2 m_pan  = { 0.0, 0.0 };

	glm::mat4 m_proj;
	glm::mat4 m_view;

	void Pan(glm::vec2 delta);
	void Zoom(glm::vec2 p, float factor);

	wxDECLARE_EVENT_TABLE();
};

#endif