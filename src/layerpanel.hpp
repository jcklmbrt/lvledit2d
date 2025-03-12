#include <bitset>
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/listctrl.h>

#include "src/singleton.hpp"

class LayerList;

class LayerPanel : public wxPanel,
                   public Singleton<LayerPanel>
{
public:
	LayerPanel(wxWindow *parent);
private:
	void OnNew(wxCommandEvent &e);
	void OnDel(wxCommandEvent &e);
};

class LayerList : public wxListCtrl,
                  public Singleton<LayerList>
{
	enum ColumnID
	{
		NAME,
		POLYGONS
	};
public:
	LayerList(wxWindow *parent);
private:
	wxString OnGetItemText(long item, long col) const;
};
