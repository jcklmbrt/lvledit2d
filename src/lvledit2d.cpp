#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "src/mainframe.hpp"
#include "src/lvledit2d.hpp"


bool LvlEdit2D::OnInit()
{
	if(!wxApp::OnInit()) {
		return false;
	}
	m_mainframe = new MainFrame;
	m_mainframe->Show(true);
	return true;
}


int LvlEdit2D::OnExit()
{
	return wxApp::OnExit();
}


wxFrame *LvlEdit2D::GetMainFrame()
{
	return m_mainframe;
}

wxIMPLEMENT_APP(LvlEdit2D);
