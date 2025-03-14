#include "src/viewmatrix.hpp"
#include "src/texturepanel.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/textureedit.hpp"

TextureEdit::TextureEdit(GLCanvas *canvas)
	: IBaseEdit(canvas)
{
	Bind(wxEVT_LEFT_DOWN, &TextureEdit::OnMouseLeftDown, this);
	Bind(wxEVT_RIGHT_DOWN, &TextureEdit::OnMouseRightDown, this);
};

void TextureEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	glm::vec2 wpos = m_canvas->view.MouseToWorld(e);
	ConvexPolygon *poly = m_canvas->editor.FindPoly(wpos);
	TextureList *tlist = TextureList::GetInstance();
	TexturePanel *tpanel = TexturePanel::GetInstance();

	m_canvas->editor.SetSelectedPoly(poly);

	if(poly != nullptr && tlist->GetSelected() != -1) {
		EditAction_Texture act;
		act.index = tlist->GetSelected();
		act.scale = tpanel->GetSliderValue();
		m_canvas->editor.AppendAction(act);
	}

	m_canvas->Refresh(true);

	e.Skip();
}

void TextureEdit::OnMouseRightDown(wxMouseEvent &e)
{
	glm::vec2 wpos = m_canvas->view.MouseToWorld(e);
	ConvexPolygon *poly = m_canvas->editor.FindPoly(wpos);
	TexturePanel *tpanel = TexturePanel::GetInstance();

	m_canvas->editor.SetSelectedPoly(poly);

	if(poly != nullptr && tpanel != nullptr) {
		EditAction_Texture act;
		act.index = -1;
		act.scale = 0;
		m_canvas->editor.AppendAction(act);
	}

	m_canvas->Refresh(true);

	e.Skip();
}

