#ifndef _TEXTURELIST_HPP
#define _TEXTURELIST_HPP


#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/filename.h>


#include "src/singleton.hpp"


class wxWindow;
class wxItemAttr;
class TextureList;
class wxStaticText;
class wxSlider;

class TexturePanel : public wxPanel,
                     public Singleton<TexturePanel>
{
public:
	TexturePanel(wxWindow *parent);
	void OnOpen(wxCommandEvent &e);
	void OnSlider(wxCommandEvent &e);
	int GetSliderValue();
private:
	wxSlider *m_slider;
	wxStaticText *m_label;
};


class TextureList : public wxListCtrl,
                    public Singleton<TextureList>
{
	static constexpr int ICON_SIZE = 32;
	enum ColumnID 
	{
		PREVIEW,
		NAME,
		SIZE,
		TYPE
	};
public:
	TextureList(wxWindow *parent);
	wxString OnGetItemText(long item, long col) const;
	int OnGetItemImage(long item) const;
	size_t GetSelected();
};


#endif