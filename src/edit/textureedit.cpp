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
	Point2D wpos = View.MouseToWorld(e);
	ConvexPolygon *poly = Context->SelectPoly(wpos);
	TextureList *tlist = TextureList::GetInstance();
	TexturePanel *tpanel = TexturePanel::GetInstance();

	if(poly != nullptr) {
		EditAction_Texture act;
		act.Index = tlist->GetSelected();
		act.Scale = tpanel->GetSliderValue();
		Context->AppendAction(act);
	}

	Canvas->Refresh(true);

	e.Skip();
}

void TextureEdit::OnMouseRightDown(wxMouseEvent &e)
{
	Point2D wpos = View.MouseToWorld(e);
	ConvexPolygon *poly = Context->SelectPoly(wpos);
	TexturePanel *tpanel = TexturePanel::GetInstance();

	if(poly != nullptr && tpanel != nullptr) {
		EditAction_Texture act;
		act.Index = -1;
		act.Scale = 0;
		Context->AppendAction(act);
	}

	Canvas->Refresh(true);

	e.Skip();
}

