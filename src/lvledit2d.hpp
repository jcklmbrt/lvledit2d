#ifndef _LVLEDIT2D_HPP
#define _LVLEDIT2D_HPP

#include <wx/wx.h>

class LvlEdit2D : public wxApp
{
public:
	virtual bool OnInit();
	virtual int  OnExit();
private:
	wxFrame *m_mainframe = nullptr;
};

DECLARE_APP(LvlEdit2D)

#endif