#ifndef _IBASEEDIT_HPP
#define _IBASEEDIT_HPP


#include <wx/geometry.h>
#include <wx/wx.h>
#include "src/drawpanel.hpp"


class IBaseEdit
{
public:
	IBaseEdit(DrawPanel *parent);
	virtual ~IBaseEdit();
	virtual void OnMouseLeftDown(wxMouseEvent &e) = 0;
	virtual void OnMouseLeftUp(wxMouseEvent &e)   = 0;
	virtual void OnMouseMotion(wxMouseEvent &e)   = 0;
	virtual void OnPaint(wxPaintDC &dc) = 0;
	DrawPanel *GetParent();
protected:
	DrawPanel *m_parent;
};


inline IBaseEdit::IBaseEdit(DrawPanel *parent) 
	: m_parent(parent)
{
	wxASSERT(parent);
}


inline IBaseEdit::~IBaseEdit()
{

}


inline DrawPanel *IBaseEdit::GetParent()
{
	return m_parent;
}


#endif
