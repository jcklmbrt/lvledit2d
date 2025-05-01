#ifndef _SINGLESELECTLIST_HPP
#define _SINGLESELECTLIST_HPP

#include <wx/listctrl.h>

class SingleSelectList : public wxListCtrl
{
protected:
	SingleSelectList(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxLC_ICON,
		const wxValidator &validator = wxDefaultValidator,
		const wxString &name = wxASCII_STR(wxListCtrlNameStr));

	virtual void OnSetSelected(long item) = 0;
private:
	void OnLeftDown(wxMouseEvent &e);
	void OnMouse(wxMouseEvent &e);
	void OnSelected(wxListEvent &e);
	void UnselectAll();
};

#endif