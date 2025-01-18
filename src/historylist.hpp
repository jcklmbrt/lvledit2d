#ifndef _HISTORYLIST_HPP
#define _HISTORYLIST_HPP

#include "src/drawpanel.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/mainframe.hpp"
#include "src/lvledit2d.hpp"
#include <wx/listbase.h>
#include <wx/listctrl.h>
#include <wx/string.h>

class HistoryList : public wxListCtrl
{
	enum ColumnID : long
	{
		ACTION,
		VALUE
	};
public:
	HistoryList(wxWindow *parent)
		: wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxLC_REPORT | wxLC_VIRTUAL | wxLC_VRULES | wxLC_HRULES)
	{
		wxSize size = GetSize();
		/* full width column */
		InsertColumn(ColumnID::ACTION, "Action", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
		InsertColumn(ColumnID::VALUE,  "Value", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
		SetItemCount(0);
	}

	wxItemAttr *OnGetItemAttr(long item) const
	{
		static wxItemAttr attr;
		attr.SetBackgroundColour(*wxYELLOW);

		DrawPanel *dp = DrawPanel::GetCurrent();
		if(dp != nullptr) {
			if(dp->GetEditor().HistorySize() > item) {
				attr.SetBackgroundColour(*wxWHITE);
			}
		}

		return &attr;
	}


	wxString OnGetItemText(long item, long col) const
	{
		DrawPanel *dp = DrawPanel::GetCurrent();

		wxString s;

		if(dp != nullptr) {
			std::vector<EditAction> history = dp->GetEditor().GetHistory();
			wxASSERT(item >= 0 && item <= history.size());
			const EditAction &action = history[item];
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
				}
			} else if(col == ColumnID::VALUE) {
				switch(action.base.type) {
				case EditActionType_t::LINE:
					s.Printf("x0:%.1lf,y0:%.1lf,x1:%.1lf,y1:%.1lf", action.line.start.m_x, action.line.start.m_y, action.line.end.m_x, action.line.end.m_y);
					break;
				case EditActionType_t::RECT:
					s.Printf("x:%.1lf,y:%.1lf,w:%.1lf,h:%.1lf", action.rect.rect.m_x, action.rect.rect.m_y, action.rect.rect.m_width, action.rect.rect.m_height);
					break;
				case EditActionType_t::MOVE:
					s.Printf("x:%.1lf,y:%.1lf", action.move.delta.m_x,
						action.move.delta.m_y);
					break;
				}
			}
		}

		return s;
	}
};

#endif
