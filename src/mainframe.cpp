
#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "src/mainframe.hpp"
#include "src/glcanvas.hpp"

#include "res/icon.xpm"
#include "res/line.xpm"
#include "res/quad.xpm"
#include "res/texture.xpm"
#include "res/reset.xpm"
#include "res/pawn.xpm"
#include "res/hand.xpm"
#include "res/save.xpm"

MainFrame::MainFrame()
	: wxFrame(nullptr, wxID_ANY, "lvledit2d")
{
	SetIcon(icon_xpm);

	wxMenu *file = new wxMenu;
	file->Append(wxID_EXIT);

	wxMenuBar *menubar = new wxMenuBar;
	menubar->Append(file, "&File");

	SetMenuBar(menubar);

	wxStatusBar *status = CreateStatusBar();

	m_canvas = nullptr;
	wxGLAttributes attrs;
	attrs.PlatformDefaults().Defaults().EndList();
	bool accepted = wxGLCanvas::IsDisplaySupported(attrs);

	if(accepted) {
		m_canvas = new GLCanvas(this, attrs);
	}

	wxToolBar *toolbar = CreateToolBar();
	toolbar->AddTool(1000, "Select", hand_xpm);
	toolbar->AddTool(1001, "Line", line_xpm);
	toolbar->AddTool(1002, "Quad", quad_xpm);
	toolbar->AddTool(1003, "Texture", texture_xpm);
	toolbar->AddTool(1004, "Entity", pawn_xpm);
	toolbar->AddTool(1005, "Save", save_xpm);
	toolbar->AddTool(1005, "Reset", reset_xpm);
	toolbar->Realize();


	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
}

void MainFrame::OnExit(wxCommandEvent &e)
{
	Close(true);
}