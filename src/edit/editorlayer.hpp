#ifndef _EDITORLAYER_HPP
#define _EDITORLAYER_HPP

#include "src/geometry.hpp"

#define MAX_LAYER_NAME 32

class EditorLayer
{
public:
	EditorLayer(const char *name);
	void SetName(const char *name);
private:
	char m_name[MAX_LAYER_NAME] = { 0 };
	bool m_visible = true;
	std::vector<ConvexPolygon> m_polys;
public:
	inline std::vector<ConvexPolygon> &GetPolys() { return m_polys; }
	inline const char *GetName() { return m_name; }
};

#endif
