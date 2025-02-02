#include <wx/wx.h>
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
	wxStaticText *label = new wxStaticText(this, wxID_ANY, wxEmptyString);
	wxSlider *slider = new wxSlider(this, wxID_ANY, 0, 0, 10);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(open, 0, wxEXPAND);
	sizer->Add(label, 0, wxEXPAND);
	sizer->Add(slider, 0, wxEXPAND);
	sizer->Add(list, 1, wxEXPAND);
	SetSizer(sizer);

	m_label = label;
	m_slider = slider;

	SetScaleLabel(m_label, m_slider->GetValue());

	Bind(wxEVT_BUTTON, &TexturePanel::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_SLIDER, &TexturePanel::OnSlider, this);
}


void TexturePanel::OnSlider(wxCommandEvent &e)
{
	SetScaleLabel(m_label, m_slider->GetValue());
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

int TexturePanel::GetSliderValue()
{
	return m_slider->GetValue();
}


TextureList::TextureList(wxWindow *parent)
	: wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxLC_VIRTUAL | wxLC_REPORT | wxLC_SINGLE_SEL)
{
	wxImageList *imglist = new wxImageList(ICON_SIZE, ICON_SIZE);
	AssignImageList(imglist, wxIMAGE_LIST_SMALL);
	InsertColumn(ColumnID::PREVIEW, "Preview", wxLIST_FORMAT_CENTER, ICON_SIZE + 8);
	InsertColumn(ColumnID::NAME, "Name", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	InsertColumn(ColumnID::SIZE, "Size", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	InsertColumn(ColumnID::TYPE, "Type", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);

}


wxString TextureList::OnGetItemText(long item, long col) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	std::vector<Texture> &textures = canvas->GetEditor().GetTextures();
	const Texture &texture = textures[item];

	wxString s;

	switch(col) {
	case ColumnID::PREVIEW:
		return wxEmptyString;
	case ColumnID::NAME:
		return texture.GetFileName().GetName();
	case ColumnID::SIZE:
		s.Printf("%llux%llu", texture.GetWidth(), texture.GetHeight());
		return s;
	case ColumnID::TYPE:
		return texture.GetFileName().GetExt();
	}

	return wxEmptyString;
};


int TextureList::OnGetItemImage(long item) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	std::vector<Texture> &textures = canvas->GetEditor().GetTextures();
	return textures[item].GetIndex();
}


size_t TextureList::GetSelected()
{
	long item = -1;
	while(true) {
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if(item == -1) {
			return -1;
		} else {
			return item;
		}
	}
}