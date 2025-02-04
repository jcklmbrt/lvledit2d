#ifndef _TEXTURELIST_HPP
#define _TEXTURELIST_HPP

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/listctrl.h>


#include "src/singleton.hpp"


struct TexturePanel : public wxPanel,
                      public Singleton<TexturePanel>
{
	TexturePanel(wxWindow *parent);
	void OnOpen(wxCommandEvent &e);
	void OnSlider(wxCommandEvent &e);
	wxSlider *slider;
	wxStaticText *text;
};

#define ICON_SIZE 32

class TextureList : public wxListCtrl,
                    public Singleton<TextureList>
{
	enum ColumnID 
	{
		PREVIEW,
		NAME,
		SIZE
	};
public:
	TextureList(wxWindow *parent);
	wxString OnGetItemText(long item, long col) const;
	int OnGetItemImage(long item) const;
	size_t GetSelected();
};


#endif
