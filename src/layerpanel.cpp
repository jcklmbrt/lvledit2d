
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/clrpicker.h>

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
	m_clrpicker = new wxColourPickerCtrl(this, wxID_ANY);
	m_clrpicker->SetColour(*wxGREEN);
	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_clrpicker, 0, wxEXPAND | wxALL);
	sizer->Add(ctrl, 0, wxEXPAND | wxALL);
	sizer->Add(list, 1, wxEXPAND);
	SetSizer(sizer);

	Bind(wxEVT_BUTTON, &LayerPanel::OnNew, this, wxID_NEW);
	Bind(wxEVT_BUTTON, &LayerPanel::OnDel, this, wxID_DELETE);
}

wxColour LayerPanel::GetColor() const
{
	return m_clrpicker->GetColour();
}

void LayerPanel::OnNew(wxCommandEvent &e)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas == nullptr) {
		return;
	}

	EditorContext &edit = canvas->GetEditor();

	ActLayer act;
	act.color = GetColor().GetRGB();
	edit.AddAction(act);

	Refresh();
}

void LayerPanel::OnDel(wxCommandEvent &e)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas == nullptr) {
		return;
	}

	EditorContext &edit = canvas->GetEditor();
	edit.DeleteLayer();

	LayerList *list = LayerList::GetInstance();
	list->SetItemCount(edit.GetLayers().size());

	canvas->Refresh();
	Refresh();
}

LayerList::LayerList(wxWindow *parent)
	: SingleSelectList(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxLC_VIRTUAL | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES)
{
	InsertColumn(ColumnID::NAME, "Name", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	InsertColumn(ColumnID::POLYGONS, "Polygons", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	SetItemCount(0);
	Show();
}

void LayerList::OnSetSelected(long item)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas != nullptr) {
		EditorContext &edit = canvas->GetEditor();
		edit.SetSelectedLayerIndex(item);
		canvas->Refresh();
	}
}

wxString LayerList::OnGetItemText(long item, long col) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas == nullptr) {
		return wxEmptyString;
	}
	const std::vector<EditorLayer> &layers = canvas->GetEditor().GetLayers();
	const EditorLayer &layer = layers[item];

	wxString s;

	switch(col) {
	case ColumnID::NAME:
		s.Printf("%ld", item);
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
		EditorContext &edit = canvas->GetEditor();
		if(edit.GetSelectedLayer() == &edit.GetLayers()[item]) {
			attr.SetBackgroundColour(sel);
			attr.SetTextColour(*wxWHITE);
		}
	}

	return &attr;
}