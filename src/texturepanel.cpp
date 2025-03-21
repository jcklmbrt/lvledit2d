#include <wx/listbase.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include "src/gl/texture.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/texturepanel.hpp"


static void SetScaleLabel(wxStaticText *label, int scale)
{
	wxString s;
	if(scale == 0) {
		s.Printf("Texture Scale: Fit Bounding Box");
	} else {
		s.Printf("Texture Scale: %d x Background Grid", scale);
	}

	label->SetLabel(s);
}


TexturePanel::TexturePanel(wxWindow *parent) 
	: wxPanel(parent)
{
	wxListCtrl *list = new TextureList(this);
	wxButton *open = new wxButton(this, wxID_OPEN, "Open");

	wxPanel *ctrl = new wxPanel(this);
	m_text = new wxStaticText(ctrl, wxID_ANY, wxEmptyString);
	m_slider = new wxSlider(ctrl, wxID_ANY, 0, 0, 10);
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(m_text, 0, wxEXPAND);
	sizer->Add(m_slider, 1, wxEXPAND);
	ctrl->SetSizer(sizer);

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(open, 0, wxEXPAND);
	sizer->Add(ctrl, 0, wxEXPAND | wxALL);
	sizer->Add(list, 1, wxEXPAND);
	SetSizer(sizer);

	SetScaleLabel(m_text, m_slider->GetValue());

	Bind(wxEVT_BUTTON, &TexturePanel::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_SLIDER, &TexturePanel::OnSlider, this);
}


void TexturePanel::OnSlider(wxCommandEvent &e)
{
	SetScaleLabel(m_text, m_slider->GetValue());
}


void TexturePanel::OnOpen(wxCommandEvent &e)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();

	if(canvas == nullptr) {
		return;
	}

	wxFileDialog dialog(this, "Open Image File", wxEmptyString, wxEmptyString,
		"Image files (*.png;*.jpg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if(dialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	canvas->GetEditor().AddTexture(dialog.GetPath());
}

TextureList::TextureList(wxWindow *parent)
	: wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxLC_VIRTUAL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES)
{
	wxImageList *imglist = new wxImageList(ICON_SIZE, ICON_SIZE);
	AssignImageList(imglist, wxIMAGE_LIST_SMALL);
	InsertColumn(ColumnID::PREVIEW, "Preview", wxLIST_FORMAT_LEFT, ICON_SIZE + 8);
	InsertColumn(ColumnID::NAME, "Name", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	InsertColumn(ColumnID::SIZE, "Size", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
#ifndef _WIN32
	wxSize size = GetSize() * 0.7;
	wxListItem col;
	GetColumn(ColumnID::NAME, col);
	col.SetWidth(size.GetWidth());
	SetColumn(ColumnID::NAME, col);
#endif
	Bind(wxEVT_LIST_ITEM_FOCUSED, &TextureList::OnFocused, this);
	Bind(wxEVT_LIST_ITEM_SELECTED, &TextureList::OnSelected, this);
}


void TextureList::OnFocused(wxListEvent &e)
{
	for(long i = 0; i < GetItemCount(); i++) {
		//if(GetItemState(i, wxLIST_STATE_FOCUSED)) {
		//	m_selected = i;
		//}
		SetItemState(i, 0, wxLIST_STATE_FOCUSED);
	}
	e.Skip();
	Refresh(true);
}

void TextureList::OnSelected(wxListEvent &e)
{
	for(long i = 0; i < GetItemCount(); i++) {
		if(GetItemState(i, wxLIST_STATE_SELECTED)) {
			m_selected = i;
		}
		SetItemState(i, 0, wxLIST_STATE_SELECTED);
	}
	e.Skip();
	Refresh(true);
}


wxString TextureList::OnGetItemText(long item, long col) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	wxASSERT(canvas);

	const GLTexture &texture = canvas->GetEditor()
		.GetTextures()
		.at(item);

	wxString s;

	switch(col) {
	case ColumnID::PREVIEW:
		return wxEmptyString;
	case ColumnID::NAME:
		return texture.GetName();
	case ColumnID::SIZE:
		s.Printf("%llux%llu", texture.GetWidth(), texture.GetHeight());
		return s;
	}

	return wxEmptyString;
};

wxItemAttr *TextureList::OnGetItemAttr(long item) const 
{
	static wxItemAttr attr;
	wxColor sel = wxColor(100, 100, 255);
	if(item == m_selected) {
		attr.SetBackgroundColour(sel);
		attr.SetTextColour(*wxWHITE);
	} else {
		attr.SetBackgroundColour(*wxWHITE);
		attr.SetTextColour(*wxBLACK);
	}
	return &attr;
}


int TextureList::OnGetItemImage(long item) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	wxASSERT(canvas);
	return canvas->GetEditor()
		.GetTextures()
		.at(item)
		.GetThumb();
}
