#ifndef _IBASEEDIT_HPP
#define _IBASEEDIT_HPP

#include <wx/event.h>
#include <wx/geometry.h>
#include <wx/wx.h>
#include "src/drawpanel.hpp"


class IBaseEdit : public wxEvtHandler
{
public:
	IBaseEdit(DrawPanel *parent);
	virtual ~IBaseEdit();
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
