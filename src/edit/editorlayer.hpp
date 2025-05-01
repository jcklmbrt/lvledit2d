#ifndef _EDITORLAYER_HPP
#define _EDITORLAYER_HPP

#include <wx/wx.h>
#include "src/geometry.hpp"

class EditorLayer
{
public:
	EditorLayer(const wxColour &color);
	void RemovePoly(size_t index);
	void AddPoly(size_t index);
private:
	wxColour m_color;
	std::vector<size_t> m_polys;
public:
	inline const std::vector<size_t> &GetPolys() const { return m_polys; }
	inline std::vector<size_t> &GetPolys() { return m_polys; }
};

#endif
