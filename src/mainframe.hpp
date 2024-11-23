#ifndef _MAINFRAME_HPP
#define _MAINFRAME_HPP

#include <wx/wx.h>
#include <wx/glcanvas.h>

class MainFrame : public wxFrame
{
public:
	MainFrame();
private:
	void OnExit(wxCommandEvent &e);

	wxGLCanvas *m_canvas = nullptr;
};

#endif