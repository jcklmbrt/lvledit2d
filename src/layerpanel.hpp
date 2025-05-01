#include <bitset>
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/listctrl.h>

#include "src/singleselectlist.hpp"
#include "src/util/singleton.hpp"

class LayerList;
class wxColourPickerCtrl;

class LayerPanel : public wxPanel,
                   public Singleton<LayerPanel>
{
public:
	LayerPanel(wxWindow *parent);
	wxColour GetColor() const;
private:
	void OnNew(wxCommandEvent &e);
	void OnDel(wxCommandEvent &e);
	wxColourPickerCtrl *m_clrpicker;
};

class LayerList : public SingleSelectList,
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
	wxItemAttr *OnGetItemAttr(long item) const;
	wxString OnGetItemText(long item, long col) const;
	void OnSetSelected(long item) final;
};
