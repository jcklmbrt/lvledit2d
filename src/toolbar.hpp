#ifndef _TOOLBAR_HPP
#define _TOOLBAR_HPP

#include <wx/wx.h>


class ToolBar : public wxToolBar
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
	wxToolBarToolBase *GetSelected();
private:
	void OnSelect(wxCommandEvent &e);
        ID m_selected = ID::SELECT;
};


#endif
