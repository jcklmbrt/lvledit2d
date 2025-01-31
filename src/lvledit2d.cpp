#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "src/mainframe.hpp"
#include "src/lvledit2d.hpp"


bool LvlEdit2D::OnInit()
{
	if(!wxApp::OnInit()) {
		return false;
	}

	wxInitAllImageHandlers();

	wxFrame *mainframe = new MainFrame;
	mainframe->Show(true);
	return true;
}


int LvlEdit2D::OnExit()
{
	return wxApp::OnExit();
}


wxIMPLEMENT_APP(LvlEdit2D);
