#include "src/singleselectlist.hpp"


SingleSelectList::SingleSelectList(wxWindow *parent,
	wxWindowID id,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxValidator &validator,
	const wxString &name)
	: wxListCtrl(parent, id, pos, size, style, validator, name)
{
	Bind(wxEVT_LIST_ITEM_SELECTED, &SingleSelectList::OnSelected, this);
	Bind(wxEVT_LIST_INSERT_ITEM, &SingleSelectList::OnSelected, this);
	Bind(wxEVT_LIST_COL_END_DRAG, &SingleSelectList::OnSelected, this);
	Bind(wxEVT_LIST_ITEM_FOCUSED, &SingleSelectList::OnSelected, this);
	Bind(wxEVT_LIST_ITEM_ACTIVATED, &SingleSelectList::OnSelected, this);

	Bind(wxEVT_MOTION, &SingleSelectList::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &SingleSelectList::OnMouse, this);
	Bind(wxEVT_LEFT_DOWN, &SingleSelectList::OnLeftDown, this);
	Bind(wxEVT_RIGHT_UP, &SingleSelectList::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, &SingleSelectList::OnMouse, this);
}

void SingleSelectList::OnLeftDown(wxMouseEvent &e) 
{
	int flags = wxLIST_HITTEST_ONITEM;
	long item = HitTest(e.GetPosition(), flags);
	if(item != wxNOT_FOUND) {
		OnSetSelected(item);
	}
	UnselectAll();
	e.Skip(false);
}

void SingleSelectList::OnMouse(wxMouseEvent &e)
{
	UnselectAll();
	e.Skip(false);
}

void SingleSelectList::OnSelected(wxListEvent &e)
{
	for(long i = 0; i < GetItemCount(); i++) {
		if(GetItemState(i, wxLIST_STATE_SELECTED)) {
			OnSetSelected(i);
		}
	}
	UnselectAll();
	e.Skip(false);
}

void SingleSelectList::UnselectAll()
{
	for(long i = 0; i < GetItemCount(); i++) {
		for(int b = 0; b < 8; b++) {
			SetItemState(i, 0, 1 << b);
		}
	}
	Refresh(true);
}