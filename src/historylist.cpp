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
		Point2D lt, rb;
		const EditAction &action = dp->editor.actions[item];
		const Matrix3 &t = action.trans.matrix;
		if(col == ColumnID::ACTION) {
			switch(action.base.type) {
			case EditActionType::LINE:
				s.Printf("LINE");
				break;
			case EditActionType::RECT:
				s.Printf("RECT");
				break;
			case EditActionType::TRANS:
				s.Printf("TRANS");
				break;
			case EditActionType::TEXTURE:
				s.Printf("TEXTURE");
				break;
			case EditActionType::DELETE:
				s.Printf("DELETE");
				break;
			}
		} else if(col == ColumnID::VALUE) {
			switch(action.base.type) {
			case EditActionType::LINE:
				s.Printf("x0:%.1f,y0:%.1f,x1:%.1f,y1:%.1f", action.line.start.x, action.line.start.y, action.line.end.x, action.line.end.y);
				break;
			case EditActionType::RECT:
				lt = action.rect.rect.GetLeftTop();
				rb = action.rect.rect.GetRightBottom();
				s.Printf("%llu: x:%.1f,y:%.1f,w:%.1f,h:%.1f",
					 action.base.poly,
					 lt.x, lt.y, lt.x + rb.x, lt.y + rb.y);
				break;
			case EditActionType::TRANS:
				s.Printf("%.1f, %.1f, %.1f, "
					 "%.1f, %1.f, %.1f, "
					 "%.1f, %1.f, %.1f",
					 t[0][0], t[0][1], t[0][2],
					 t[1][0], t[1][1], t[1][2],
					 t[2][0], t[2][1], t[2][2]);
				break;
			case EditActionType::TEXTURE:
				s.Printf("index: %llu, scale: %d", action.texture.index,
					 action.texture.scale);
				break;
			case EditActionType::DELETE:
				s.Printf("index: %llu", action.base.poly);
			}
		}
	}
	return s;
}
