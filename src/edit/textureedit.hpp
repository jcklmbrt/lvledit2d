#ifndef _TEXTUREEDIT_HPP
#define _TEXTUREEDIT_HPP

#include "src/edit/editorcontext.hpp"

class TextureEdit : public IBaseEdit
{
public:
	TextureEdit(GLCanvas *panel);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseRightDown(wxMouseEvent &e);
};

#endif