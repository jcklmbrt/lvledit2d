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
	Point2D wpos = canvas->view.MouseToWorld(e);
	ConvexPolygon *poly = canvas->editor.SelectPoly(wpos);
	TextureList *tlist = TextureList::GetInstance();
	TexturePanel *tpanel = TexturePanel::GetInstance();

	if(poly != nullptr) {
		EditAction_Texture act;
		act.index = tlist->GetSelected();
		act.scale = tpanel->slider->GetValue();
		canvas->editor.AppendAction(act);
	}

	canvas->Refresh(true);

	e.Skip();
}

void TextureEdit::OnMouseRightDown(wxMouseEvent &e)
{
	Point2D wpos = canvas->view.MouseToWorld(e);
	ConvexPolygon *poly = canvas->editor.SelectPoly(wpos);
	TexturePanel *tpanel = TexturePanel::GetInstance();

	if(poly != nullptr && tpanel != nullptr) {
		EditAction_Texture act;
		act.index = -1;
		act.scale = 0;
		canvas->editor.AppendAction(act);
	}

	canvas->Refresh(true);

	e.Skip();
}

