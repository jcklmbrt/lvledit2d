#ifndef _IBASEEDIT_HPP
#define _IBASEEDIT_HPP

#include <wx/event.h>

class GLCanvas;
class EditorContext;
class ViewMatrixBase;
class ConvexPolygon;

class IBaseEdit : public wxEvtHandler
{
public:
	IBaseEdit(GLCanvas *canvas);
	virtual void DrawPolygon(const ConvexPolygon *p);
	virtual void OnDraw();
	virtual ~IBaseEdit();
protected:
	GLCanvas *m_canvas;
	EditorContext &m_edit;
	const ViewMatrixBase &m_view;
};


#endif