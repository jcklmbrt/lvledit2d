#include "src/viewmatrix.hpp"
#include "src/texturepanel.hpp"
#include "src/glcanvas.hpp"
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
	Point2D wpos = m_view.MouseToWorld(e);
	ConvexPolygon *poly = m_context->SelectPoly(wpos);
	TextureList *tlist = TextureList::GetInstance();
	TexturePanel *tpanel = TexturePanel::GetInstance();

	if(poly != nullptr) {
		poly->SetTexture(tlist->GetSelected());
		poly->SetScale(tpanel->GetSliderValue());
	}

	m_canvas->Refresh(true);

	e.Skip();
}

void TextureEdit::OnMouseRightDown(wxMouseEvent &e)
{
	Point2D wpos = m_view.MouseToWorld(e);
	ConvexPolygon *poly = m_context->SelectPoly(wpos);
	TexturePanel *tpanel = TexturePanel::GetInstance();

	if(poly != nullptr && tpanel != nullptr) {
		poly->SetTexture(-1);
		poly->SetScale(tpanel->GetSliderValue());
	}

	m_canvas->Refresh(true);

	e.Skip();
}

