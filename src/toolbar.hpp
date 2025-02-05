#ifndef _TOOLBAR_HPP
#define _TOOLBAR_HPP

#include "src/singleton.hpp"
#include <wx/wx.h>


class ToolBar : public wxToolBar,
                public Singleton<ToolBar>
{
public:
	enum ID : int 
	{
		MASK = 0x80,
		SELECT,
		LINE,
		QUAD,
		TEXTURE,
		ENTITY
	};
	ToolBar(wxWindow *parent, wxWindowID id = wxID_ANY);
	void OnSelect(wxCommandEvent &e);
        ID selected = ID::SELECT;
};


#endif
