#ifndef _TEXTURELIST_HPP
#define _TEXTURELIST_HPP

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/listctrl.h>

#include "src/util/singleton.hpp"
#include "src/singleselectlist.hpp"

struct TexturePanel : public wxPanel,
                      public Singleton<TexturePanel>
{
	TexturePanel(wxWindow *parent);
	void OnOpen(wxCommandEvent &e);
	void OnDelete(wxCommandEvent &e);
	void OnSlider(wxCommandEvent &e);
private:
	wxSlider *m_slider;
	wxStaticText *m_text;
public:
	int GetSliderValue() { return m_slider->GetValue(); }
};


class TextureList : public SingleSelectList,
                    public Singleton<TextureList>
{
	enum ColumnID 
	{
		PREVIEW,
		NAME,
		SIZE
	};
public:
	static constexpr int ICON_SIZE = 24;
	TextureList(wxWindow *parent);
	wxString OnGetItemText(long item, long col) const;
	int OnGetItemImage(long item) const;
	wxItemAttr *OnGetItemAttr(long item) const;
private:
	void OnSetSelected(long item) final;
};

#endif
