#include "src/viewmatrix.hpp"
#include "src/texturepanel.hpp"
#include "src/glcanvas.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/textureedit.hpp"

TextureEdit::TextureEdit(GLCanvas *canvas)
	: IBaseEdit(canvas)
{
	Bind(wxEVT_LEFT_DOWN, &TextureEdit::OnMouseLeftDown, this);
	Bind(wxEVT_MOTION, &TextureEdit::OnMouseMotion, this);
	Bind(wxEVT_LEFT_UP, &TextureEdit::OnMouseLeftUp, this);

	m_context->SetSelectedPoly(nullptr);
};

void TextureEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	Point2D wpos = m_view.MouseToWorld(e);
	ConvexPolygon *poly = m_context->SelectPoly(wpos);
	TextureList *tlist = TextureList::GetInstance();

	if(poly != nullptr && tlist != nullptr) {
		poly->SetTexture(tlist->GetSelected());
	}

	m_canvas->Refresh(true);

	e.Skip();
}


void TextureEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip();
}


void TextureEdit::OnMouseLeftUp(wxMouseEvent &e)
{
	e.Skip();
}