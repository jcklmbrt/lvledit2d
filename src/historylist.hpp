#ifndef _HISTORYLIST_HPP
#define _HISTORYLIST_HPP

#include <wx/listctrl.h>

class wxWindow;
class wxItemAttr;

class HistoryList : public wxListCtrl
{
	enum ColumnID : long
	{
		ACTION,
		VALUE
	};
public:
	HistoryList(wxWindow *parent);
	wxItemAttr *OnGetItemAttr(long item) const;
	wxString OnGetItemText(long item, long col) const;
};

#endif
