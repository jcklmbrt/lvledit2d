
#include <wx/panel.h>
#include <wx/sizer.h>

#include "src/gl/glcanvas.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/layerpanel.hpp"

LayerPanel::LayerPanel(wxWindow *parent)
	: wxPanel(parent)
{
	wxPanel *ctrl = new wxPanel(this);
	wxButton *newlayer = new wxButton(ctrl, wxID_NEW, "New Layer");
	wxButton *dellayer = new wxButton(ctrl, wxID_DELETE, "Delete Layer");

	wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(newlayer, 1, wxEXPAND);
	sizer->Add(dellayer, 1, wxEXPAND);
	ctrl->SetSizer(sizer);

	LayerList *list = new LayerList(this);

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(ctrl, 0, wxEXPAND | wxALL);
	sizer->Add(list, 1, wxEXPAND);
	SetSizer(sizer);

	Bind(wxEVT_BUTTON, &LayerPanel::OnNew, this, wxID_NEW);
	Bind(wxEVT_BUTTON, &LayerPanel::OnDel, this, wxID_DELETE);
}

void LayerPanel::OnNew(wxCommandEvent &e)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas == nullptr) {
		return;
	}

	EditorContext &edit = canvas->GetEditor();

	std::vector<EditorLayer> &layers = edit.GetLayers();
	layers.push_back("test");

	LayerList *list = LayerList::GetInstance();
	list->SetItemCount(layers.size());

	edit.SetSelectedLayer(&layers.back());

	Refresh();
}

void LayerPanel::OnDel(wxCommandEvent &e)
{
	Refresh();
}

LayerList::LayerList(wxWindow *parent)
	: wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxLC_VIRTUAL | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES)
{
	InsertColumn(ColumnID::NAME, "Name", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	InsertColumn(ColumnID::POLYGONS, "Polygons", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	SetItemCount(0);

	Bind(wxEVT_LIST_ITEM_FOCUSED, &LayerList::OnFocused, this);
	Bind(wxEVT_LIST_ITEM_SELECTED, &LayerList::OnSelected, this);
}


void LayerList::OnSelected(wxListEvent &e)
{
	for(long i = 0; i < GetItemCount(); i++) {
		if(GetItemState(i, wxLIST_STATE_SELECTED)) {
			GLCanvas *canvas = GLCanvas::GetCurrent();
			if(canvas != nullptr) {
				canvas->GetEditor().SetSelectedLayerIndex(i);
			}
		}
		SetItemState(i, 0, wxLIST_STATE_SELECTED);
	}
	e.Skip();
	Refresh(true);
}

void LayerList::OnFocused(wxListEvent &e)
{
	for(long i = 0; i < GetItemCount(); i++) {
		SetItemState(i, 0, wxLIST_STATE_FOCUSED);
	}
	e.Skip();
	Refresh(true);
}

wxString LayerList::OnGetItemText(long item, long col) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas == nullptr) {
		return wxEmptyString;
	}

	EditorLayer &layer = canvas->GetEditor()
		.GetLayers()[item];

	wxString s;

	switch(col) {
	case ColumnID::NAME:
		s.Printf("%s", layer.GetName());
		break;
	case ColumnID::POLYGONS:
		s.Printf("%llu", layer.GetPolys().size());
		break;
	}
	
	return s;
}	

wxItemAttr *LayerList::OnGetItemAttr(long item) const
{
	static wxItemAttr attr;

	wxColor sel = wxColor(100, 100, 255);
	attr.SetBackgroundColour(*wxWHITE);
	attr.SetTextColour(*wxBLACK);

	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas != nullptr) {
		if(item == canvas->GetEditor().GetSelectedLayerIndex()) {
			attr.SetBackgroundColour(sel);
			attr.SetTextColour(*wxWHITE);
		}
	}

	return &attr;
}