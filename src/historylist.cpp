#include <sstream>
#include <wx/listbase.h>
#include <wx/wx.h>
#include "src/gl/glcanvas.hpp"
#include "src/historylist.hpp"
#include "src/edit/editorcontext.hpp"


HistoryList::HistoryList(wxWindow *parent)
	: wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		     wxLC_REPORT | wxLC_VIRTUAL | wxLC_VRULES | wxLC_HRULES)
{
	/* autosize column */
	InsertColumn(ColumnID::ACTION, "Action", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	InsertColumn(ColumnID::POLY, "Poly", wxLIST_FIND_LEFT, wxLIST_AUTOSIZE);
	InsertColumn(ColumnID::LAYER, "Layer", wxLIST_FIND_LEFT, wxLIST_AUTOSIZE);
	InsertColumn(ColumnID::VALUE,  "Value", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
#ifndef _WIN32 
	wxSize size = GetSize() * 0.7;
	wxListItem valcol;
	GetColumn(ColumnID::VALUE, valcol);
	valcol.SetWidth(size.GetWidth());
	SetColumn(ColumnID::VALUE, valcol);
#endif
	SetItemCount(0);
}


wxItemAttr *HistoryList::OnGetItemAttr(long item) const
{
	static wxItemAttr attr;
	attr.SetBackgroundColour(*wxYELLOW);

	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas != nullptr) {
		if(canvas->GetEditor().GetActList().HistoryIndex() > item) {
			attr.SetBackgroundColour(*wxWHITE);
		}
	}

	return &attr;
}


wxString HistoryList::OnGetItemText(long item, long col) const
{
	GLCanvas *canvas = GLCanvas::GetCurrent();

	wxString s;

	if(canvas != nullptr) {

		EditorContext &edit = canvas->GetEditor();
		ActList &actions = edit.GetActList();
		wxASSERT(item >= 0 && item <= actions.TotalActions());
		glm::i32vec2 lt, rb;
		ActData act;
		if(!actions.GetAction(item, act)) {
			return s;
		};
		if(col == ColumnID::ACTION) {
			switch(act.type) {
			case ActType::LINE: s.Printf("LINE"); break;
			case ActType::RECT: s.Printf("RECT"); break;
			case ActType::MOVE: s.Printf("MOVE"); break;
			case ActType::SCALE: s.Printf("SCALE"); break;
			case ActType::TEXTURE: s.Printf("TEXTURE"); break;
			case ActType::DEL: s.Printf("DEL"); break;
			case ActType::LAYER: s.Printf("LAYER"); break;
			}
		} else if(col == ColumnID::VALUE) {
			switch(act.type) {
			case ActType::LINE:
				s.Printf("%dx + %dy + %d", act.line.a, act.line.b, act.line.c);
				break;
			case ActType::RECT:
				lt = act.rect.mins;
				rb = act.rect.maxs;
				s.Printf("%d %d %d %d", lt.x, lt.y, lt.x + rb.x, lt.y + rb.y);
				break;
			case ActType::MOVE:
				s.Printf("%d %d", act.move.x, act.move.y);
				break;
			case ActType::SCALE:
				s.Printf("%d/%d %d/%d",
					 act.scale.numer.x, act.scale.denom.x,
					 act.scale.numer.y, act.scale.denom.y);
				break;
			case ActType::TEXTURE:
				s.Printf("index: %d, scale: %d", 
					act.texture.index,
					act.texture.scale);
				break;
			case ActType::LAYER:
			case ActType::DEL:
				break;
			}
		} else if(col == ColumnID::POLY) {
			if(act.type != ActType::LAYER && act.poly != -1) {
				s.Printf("%u", act.poly);
			} else {
				s = wxEmptyString;
			}
		} else if(col == ColumnID::LAYER) {
			s.Printf("%u", act.layer);

		}
	}
	return s;
}
