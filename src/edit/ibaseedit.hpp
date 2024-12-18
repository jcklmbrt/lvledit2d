#ifndef _IBASEEDIT_HPP
#define _IBASEEDIT_HPP

#include <wx/geometry.h>
#include <wx/wx.h>
#include "src/drawpanel.hpp"

class IBaseEdit
{
public:
	IBaseEdit(DrawPanel *parent)
		: m_parent(parent) {}
	virtual ~IBaseEdit() {};
	virtual void OnMouseLeftDown(wxMouseEvent &e) = 0;
	virtual void OnMouseLeftUp(wxMouseEvent &e)   = 0;
	virtual void OnMouseMotion(wxMouseEvent &e)   = 0;
	virtual void OnPaint(wxPaintDC &dc) = 0;
protected:
	DrawPanel *m_parent;
};

#endif
