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
	glm::vec2 wpos = m_view.MouseToWorld(e);
	TextureList *tlist = TextureList::GetInstance();
	TexturePanel *tpanel = TexturePanel::GetInstance();

	ConvexPolygon *poly = m_edit.FindPoly(wpos);
	if(poly) {
		m_edit.SetSelectedPoly(poly);
	} else {
		poly = m_edit.GetSelectedPoly();
	}

	if(poly == nullptr) {
		return;
	}

	size_t texture_index = m_edit.GetSelectedTextureIndex();

	if(poly != nullptr && texture_index != -1) {
		ActTexture act;
		act.index = texture_index;
		act.scale = tpanel->GetSliderValue();
		m_edit.AddAction(act);
	}

	m_canvas->Refresh(true);

	e.Skip();
}

void TextureEdit::OnMouseRightDown(wxMouseEvent &e)
{
	glm::vec2 wpos = m_view.MouseToWorld(e);
	TexturePanel *tpanel = TexturePanel::GetInstance();

	if(m_edit.GetSelectedLayer() == nullptr) {
		return;
	}

	ConvexPolygon *poly = m_edit.FindPoly(wpos);

	if(m_edit.GetSelectedPoly()) {
		return;
	}

	m_edit.SetSelectedPoly(poly);

	if(poly != nullptr && tpanel != nullptr) {
		ActTexture texture;
		texture.index = -1;
		texture.scale = 0;
		m_edit.AddAction(texture);
	}

	m_canvas->Refresh(true);

	e.Skip();
}

