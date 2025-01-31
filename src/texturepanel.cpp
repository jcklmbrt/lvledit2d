#include <wx/wx.h>
#include "src/glcanvas.hpp"
#include "src/texturepanel.hpp"


TexturePanel::TexturePanel(wxWindow *parent) 
	: wxPanel(parent)
{
	wxListCtrl *list = new TextureList(this);
	wxButton *open = new wxButton(this, wxID_OPEN, "Open");

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(open, 0, wxEXPAND);
	sizer->Add(list, 1, wxEXPAND);
	SetSizer(sizer);

	Bind(wxEVT_BUTTON, &TexturePanel::OnOpen, this, wxID_OPEN);
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
		wxLC_VIRTUAL | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER)
{
	wxImageList *imglist = new wxImageList(ICON_SIZE, ICON_SIZE);
	AssignImageList(imglist, wxIMAGE_LIST_SMALL);
	InsertColumn(0, wxEmptyString, wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE_USEHEADER);
}


wxString TextureList::OnGetItemText(long item, long col) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	std::vector<Texture> &textures = canvas->GetEditor().GetTextures();
	return textures[item].GetFileName().GetName();
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