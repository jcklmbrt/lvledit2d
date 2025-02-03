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
	InsertColumn(ColumnID::VALUE,  "Value", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
#ifndef _WIN32 
	wxSize size = GetSize() * 0.5;
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
		if(dp->Editor.History > item) {
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
		wxASSERT(item >= 0 && item <= dp->Editor.Actions.size());
		Point2D lt, rb;
		const EditAction &action = dp->Editor.Actions[item];
		if(col == ColumnID::ACTION) {
			switch(action.base.type) {
			case EditActionType_t::LINE:
				s.Printf("LINE");
				break;
			case EditActionType_t::RECT:
				s.Printf("RECT");
				break;
			case EditActionType_t::MOVE:
				s.Printf("MOVE");
				break;
			case EditActionType_t::TEXTURE:
				s.Printf("TEXTURE");
				break;
			}
		} else if(col == ColumnID::VALUE) {
			switch(action.base.type) {
			case EditActionType_t::LINE:
				s.Printf("x0:%.1f,y0:%.1f,x1:%.1f,y1:%.1f", action.line.start.x, action.line.start.y, action.line.end.x, action.line.end.y);
				break;
			case EditActionType_t::RECT:
				lt = action.rect.rect.GetLeftTop();
				rb = action.rect.rect.GetRightBottom();
				s.Printf("x:%.1f,y:%.1f,w:%.1f,h:%.1f", lt.x, lt.y, lt.x + rb.x, lt.y + rb.y);
				break;
			case EditActionType_t::MOVE:
				s.Printf("x:%.1f,y:%.1f", action.move.delta.x,
					 action.move.delta.y);
				break;
			case EditActionType_t::TEXTURE:
				s.Printf("index: %llu, scale: %d", action.texture.Index,
					 action.texture.Scale);
			}
		}
	}

	return s;
}
