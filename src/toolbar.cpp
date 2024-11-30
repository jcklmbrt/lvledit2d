#include <wx/event.h>
#include <wx/wx.h>
#include "src/toolbar.hpp"
#include "src/lvledit2d.hpp"

#include "res/line.xpm"
#include "res/quad.xpm"
#include "res/texture.xpm"
#include "res/pawn.xpm"
#include "res/hand.xpm"

wxBEGIN_EVENT_TABLE(ToolBar, wxToolBar)
	EVT_TOOL(ToolBar::ID::SELECT, ToolBar::OnSelect)
	EVT_TOOL(ToolBar::ID::LINE, ToolBar::OnSelect)
	EVT_TOOL(ToolBar::ID::QUAD, ToolBar::OnSelect)
	EVT_TOOL(ToolBar::ID::TEXTURE, ToolBar::OnSelect)
	EVT_TOOL(ToolBar::ID::ENTITY, ToolBar::OnSelect)
wxEND_EVENT_TABLE()

ToolBar::ToolBar(wxWindow *parent, wxWindowID id)
	: wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_LEFT)
{
	AddRadioTool(ID::SELECT,  "Select",  hand_xpm);
	AddRadioTool(ID::LINE,    "Line",    line_xpm);
	AddRadioTool(ID::QUAD,    "Quad",    quad_xpm);
	AddRadioTool(ID::TEXTURE, "Texture", texture_xpm);
	AddRadioTool(ID::ENTITY,  "Entity",  pawn_xpm);

	Realize();

	m_selected = ID::SELECT;
	/* make select tool default, wont call events */
	ToggleTool(m_selected, true);
}

wxToolBarToolBase *ToolBar::GetSelected()
{
	int pos = GetToolPos(m_selected);
	return GetToolByPos(pos);
}


void ToolBar::OnSelect(wxCommandEvent &e)
{
	int id = e.GetId();
	m_selected = static_cast<ID>(id);
	
}
