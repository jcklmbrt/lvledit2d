#include <wx/listbase.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include "src/gl/texture.hpp"
#include "src/gl/glcanvas.hpp"
#include "wx/sstream.h"
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
	wxSizer *sizer = nullptr;
	wxListCtrl *list = new TextureList(this);

	wxPanel *buttons = new wxPanel(this);
	wxButton *open = new wxButton(buttons, wxID_OPEN, "Open");
	wxButton *del = new wxButton(buttons, wxID_DELETE, "Delete Texture");
	sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(open, 1, wxEXPAND);
	sizer->Add(del, 1, wxEXPAND);
	buttons->SetSizer(sizer);

	wxPanel *ctrl = new wxPanel(this);
	m_text = new wxStaticText(ctrl, wxID_ANY, wxEmptyString);
	m_slider = new wxSlider(ctrl, wxID_ANY, 0, 0, 10);
	sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(m_text, 0, wxEXPAND);
	sizer->Add(m_slider, 1, wxEXPAND);
	ctrl->SetSizer(sizer);

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(buttons, 0, wxEXPAND);
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

void TexturePanel::OnDelete(wxCommandEvent &e)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();

	if(canvas == nullptr) {
		return;
	}
}

TextureList::TextureList(wxWindow *parent)
	: SingleSelectList(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxLC_VIRTUAL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL)
{
	wxImageList *imglist = new wxImageList(ICON_SIZE, ICON_SIZE);
	AssignImageList(imglist, wxIMAGE_LIST_SMALL);
	InsertColumn(ColumnID::PREVIEW, "Preview", wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE);
	InsertColumn(ColumnID::NAME, "Name", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	InsertColumn(ColumnID::SIZE, "Size", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
#ifndef _WIN32
	wxSize size = GetSize() * 0.7;
	wxListItem col;
	GetColumn(ColumnID::NAME, col);
	col.SetWidth(size.GetWidth());
	SetColumn(ColumnID::NAME, col);
#endif
}


void TextureList::OnSetSelected(long item)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas != nullptr) {
		canvas->GetEditor().SetSelectedTextureIndex(item);
	}
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

	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas && canvas->GetEditor().GetSelectedTextureIndex() == item) {
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
