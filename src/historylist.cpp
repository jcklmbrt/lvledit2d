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
	InsertColumn(ColumnID::INDEX, "Index", wxLIST_FIND_LEFT, wxLIST_AUTOSIZE);
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

	GLCanvas *dp = GLCanvas::GetCurrent();
	if(dp != nullptr) {
		if(dp->editor.history > item) {
			attr.SetBackgroundColour(*wxWHITE);
		}
	}

	return &attr;
}


wxString HistoryList::OnGetItemText(long item, long col) const
{
	GLCanvas *dp = GLCanvas::GetCurrent();

	wxString s;

	if(dp != nullptr) {
		wxASSERT(item >= 0 && item <= dp->editor.actions.size());
		glm::i32vec2 lt, rb;
		const EditAction &action = dp->editor.actions[item];
		if(col == ColumnID::ACTION) {
			switch(action.base.type) {
			case EditActionType::LINE:
				s.Printf("LINE");
				break;
			case EditActionType::RECT:
				s.Printf("RECT");
				break;
			case EditActionType::MOVE:
				s.Printf("MOVE");
				break;
			case EditActionType::SCALE:
				s.Printf("SCALE");
				break;
			case EditActionType::TEXTURE:
				s.Printf("TEXTURE");
				break;
			case EditActionType::DEL:
				s.Printf("DEL");
				break;
			}
		} else if(col == ColumnID::VALUE) {
			switch(action.base.type) {
			case EditActionType::LINE:
				s.Printf("%dx + %dy + %d", action.line.plane.a, action.line.plane.b, action.line.plane.c);
				break;
			case EditActionType::RECT:
				lt = action.rect.rect.mins;
				rb = action.rect.rect.maxs;
				s.Printf("%d %d %d %d", lt.x, lt.y, lt.x + rb.x, lt.y + rb.y);
				break;
			case EditActionType::MOVE:
				s.Printf("%d %d", action.move.delta.x, action.move.delta.y);
				break;
			case EditActionType::SCALE:
				s.Printf("%d/%d %d/%d",
					 action.scale.numer.x, action.scale.denom.x,
					 action.scale.numer.y, action.scale.denom.y);
				break;
			case EditActionType::TEXTURE:
				s.Printf("index: %d, scale: %d", action.texture.index,
					 action.texture.scale);
				break;
			case EditActionType::DEL:
				break;
			}
		} else if(col == ColumnID::INDEX) {
			s.Printf("%llu", action.base.poly);
		}
	}
	return s;
}
