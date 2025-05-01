#include "src/edit/editorlayer.hpp"


EditorLayer::EditorLayer(const wxColour &color)
{
	m_color = color;
}


void EditorLayer::RemovePoly(size_t poly)
{
	std::vector<size_t>::iterator pos = std::find(m_polys.begin(), m_polys.end(), poly);
	if(pos != m_polys.end()) {
		m_polys.erase(pos);
	}
}

void EditorLayer::AddPoly(size_t index)
{
	m_polys.push_back(index);
}