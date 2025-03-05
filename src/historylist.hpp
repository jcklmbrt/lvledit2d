#ifndef _HISTORYLIST_HPP
#define _HISTORYLIST_HPP

#include <wx/listctrl.h>

#include "src/singleton.hpp"

class wxWindow;
class wxItemAttr;

class HistoryList : public wxListCtrl,
                    public Singleton<HistoryList>
{
	enum ColumnID : long
	{
		ACTION,
		INDEX,
		VALUE
	};
public:
	HistoryList(wxWindow *parent);
	wxItemAttr *OnGetItemAttr(long item) const;
	wxString OnGetItemText(long item, long col) const;
};

#endif
