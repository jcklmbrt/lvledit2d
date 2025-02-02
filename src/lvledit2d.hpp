#ifndef _LVLEDIT2D_HPP
#define _LVLEDIT2D_HPP

#include <wx/wx.h>
#include "src/mainframe.hpp"

class LvlEdit2D : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
};

DECLARE_APP(LvlEdit2D)

#endif
