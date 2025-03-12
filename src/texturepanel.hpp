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
private:
	wxSlider *m_slider;
	wxStaticText *m_text;
public:
	int GetSliderValue() { return m_slider->GetValue(); }
};


class TextureList : public wxListCtrl,
                    public Singleton<TextureList>
{
	enum ColumnID 
	{
		PREVIEW,
		NAME,
		SIZE
	};
	static constexpr int ICON_SIZE = 32;
public:
	TextureList(wxWindow *parent);
	wxString OnGetItemText(long item, long col) const;
	int OnGetItemImage(long item) const;
	wxItemAttr *OnGetItemAttr(long item) const;
	void OnSelected(wxListEvent &e);
	void OnFocused(wxListEvent &e);
private:
	size_t m_selected;
public:
	void Unselect() { m_selected = -1; }
	void SetSelected(size_t i) { m_selected = i; }
	size_t GetSelected() { return m_selected; }
};

#endif
