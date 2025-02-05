#include <wx/aui/auibook.h>
#include <wx/event.h>
#include <wx/wx.h>
#include "src/toolbar.hpp"
#include "src/lvledit2d.hpp"
#include "src/gl/glcanvas.hpp"

#include "res/line.xpm"
#include "res/quad.xpm"
#include "res/texture.xpm"
#include "res/pawn.xpm"
#include "res/hand.xpm"


ToolBar::ToolBar(wxWindow *parent, wxWindowID id)
	: wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_LEFT),
	  selected(ID::SELECT)
{
	AddRadioTool(ID::SELECT, "Select", hand_xpm);
	AddRadioTool(ID::LINE, "Line", line_xpm);
	AddRadioTool(ID::QUAD, "Quad", quad_xpm);
	AddRadioTool(ID::TEXTURE, "Texture", texture_xpm);
	AddRadioTool(ID::ENTITY, "Entity",  pawn_xpm);

	Realize();
	/* make select tool default, wont call events */
	ToggleTool(selected, true);

	Bind(wxEVT_TOOL, &ToolBar::OnSelect, this, ToolBar::ID::SELECT);
	Bind(wxEVT_TOOL, &ToolBar::OnSelect, this, ToolBar::ID::LINE);
	Bind(wxEVT_TOOL, &ToolBar::OnSelect, this, ToolBar::ID::QUAD);
	Bind(wxEVT_TOOL, &ToolBar::OnSelect, this, ToolBar::ID::TEXTURE);
	Bind(wxEVT_TOOL, &ToolBar::OnSelect, this, ToolBar::ID::ENTITY);
}


void ToolBar::OnSelect(wxCommandEvent &e)
{
	int id = e.GetId();
	selected = static_cast<ID>(id);
	GLCanvas *dp = GLCanvas::GetCurrent();

	if(dp != nullptr) {
		dp->editor.OnToolSelect(selected);
	}
}
