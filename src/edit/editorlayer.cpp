#include <cstring>
#include "src/edit/editorlayer.hpp"


EditorLayer::EditorLayer(const char *name)
{
	SetName(name);
}


void EditorLayer::SetName(const char *name)
{
	strncpy(m_name, name, MAX_LAYER_NAME);
}
