#ifndef _TEXTUREEDIT_HPP
#define _TEXTUREEDIT_HPP


class TextureEdit : public IBaseEdit
{
public:
	TextureEdit(GLCanvas *panel);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
private:
};

#endif