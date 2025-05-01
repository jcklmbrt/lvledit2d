#include "src/edit/l2dfile.hpp"
#include "src/edit/editorlayer.hpp"

#include "src/edit/editaction.hpp"

struct ActLayerName : L2dLump
{
};

struct ActListHeader
{
	uint8_t magic[2];
	uint32_t history;
	L2dLump rects;
	L2dLump lines;
	L2dLump moves;
	L2dLump scales;
	L2dLump textures;
	L2dLump indices;
};

bool ActList::Undo(ActData &act)
{
	if(m_history <= 0 || m_history > m_indices.size()) {
		return false;
	}

	GetAction(m_history - 1, act);

	m_history--;

	return true;
}

bool ActList::Redo(ActData &act)
{
	if(m_history >= m_indices.size()) {
		return false;
	}

	m_history++;

	return GetAction(m_history - 1, act);
}

void ActList::PopBack()
{
	ActIndex idx = m_indices.back();
	m_indices.pop_back();
	switch(idx.type) {
	case ActType::LINE: RemoveAction(idx, m_lines); break;
	case ActType::RECT: RemoveAction(idx, m_rects); break;
	case ActType::MOVE: RemoveAction(idx, m_moves); break;
	case ActType::SCALE: RemoveAction(idx, m_scales); break;
	case ActType::TEXTURE: RemoveAction(idx, m_textures); break;
	case ActType::DEL: /* */ break;
	}
}

bool ActList::GetBack(ActData &act)
{
	if(IsEmpty() || m_history <= 0) {
		return false;
	}

	return GetAction(m_history - 1, act);
}

void ActList::UpdateBack(const ActData &act)
{
	ClearFuture();

	wxASSERT(!m_indices.empty());
	ActIndex idx = m_indices.back();
	wxASSERT(idx.type == act.type && idx.poly == act.poly && idx.layer == act.layer);
	switch(act.type) {
	case ActType::RECT: m_rects[idx.index] = act.rect; break;
	case ActType::LINE: m_lines[idx.index] = act.line; break;
	case ActType::MOVE: m_moves[idx.index] = act.move; break;
	case ActType::SCALE: m_scales[idx.index] = act.scale; break;
	case ActType::TEXTURE: m_textures[idx.index] = act.texture; break;
	case ActType::DEL:
        case ActType::LAYER:
		break;
	}

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_indices.size());
	hlist->Refresh();
}

void ActList::ClearFuture()
{
	while(m_indices.size() > m_history) {
		PopBack();
	}
}

bool ActList::GetAction(size_t i, ActData &act) const
{
	if(i < 0 || i >= m_indices.size()) {
		return false;
	}

	GetAction(m_indices[i], act);
	return true;
}

void ActList::GetAction(const ActIndex &idx, ActData &act) const
{
	act.type = idx.type;
	act.poly = idx.poly;
	act.layer = idx.layer;
	switch(idx.type) {
	case ActType::LINE: act.line = m_lines[idx.index]; break;
	case ActType::RECT: act.rect = m_rects[idx.index]; break;
	case ActType::MOVE: act.move = m_moves[idx.index]; break;
	case ActType::SCALE: act.scale = m_scales[idx.index]; break;
	case ActType::TEXTURE:  act.texture = m_textures[idx.index]; break;
	case ActType::LAYER:
	case ActType::DEL: /* */ break;
	}
}

void ActList::AddAction(const ActRect &rect, size_t poly, size_t layer)
{
	AddAction<ActRect, ActType::RECT>(m_rects, rect, poly, layer);
}

void ActList::AddAction(const ActLine &line, size_t poly, size_t layer)
{
	AddAction<ActLine, ActType::LINE>(m_lines, line, poly, layer);
}

void ActList::AddAction(const ActMove &move, size_t poly, size_t layer)
{
	AddAction<ActMove, ActType::MOVE>(m_moves, move, poly, layer);
}

void ActList::AddAction(const ActScale &scale, size_t poly, size_t layer)
{
	AddAction<ActScale, ActType::SCALE>(m_scales, scale, poly, layer);
}

void ActList::AddAction(const ActTexture &texture, size_t poly, size_t layer)
{
	AddAction<ActTexture, ActType::TEXTURE>(m_textures, texture, poly, layer);
}

void ActList::AddAction(const ActLayer &act, size_t layer)
{
	AddAction<ActLayer, ActType::LAYER>(m_layers, act, -1, layer);
}

void ActList::AddDelete(size_t poly, size_t layer)
{
	ClearFuture();
	ActIndex idx;
	idx.poly = poly;
	idx.layer = layer;
	idx.type = ActType::DEL;
	m_indices.push_back(idx);
	m_history = m_indices.size();
	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_indices.size());
	hlist->Refresh();
}


size_t ActList::BinSize() const
{
	return m_rects.size() * sizeof(ActRect) +
		m_lines.size() * sizeof(ActLine) +
		m_moves.size() * sizeof(ActMove) +
		m_scales.size() * sizeof(ActScale) +
		m_textures.size() * sizeof(ActTexture) +
		m_indices.size() * sizeof(ActIndex) +
		sizeof(ActListHeader);
}


template<typename T>
static bool ReadLump(std::vector<T> &vec, const L2dLump &lump, const uint8_t *bytes)
{
	if(lump.size % sizeof(T) != 0) {
		return false;
	}

	vec.resize(lump.size / sizeof(T));
	memcpy(vec.data(), bytes + lump.ofs, lump.size);
	return true;
}


bool ActList::Deserialize(const std::vector<uint8_t> &bytes,
                          const std::vector<uint8_t> &strings)
{
	ActListHeader hdr;
	if(bytes.size() < sizeof(ActListHeader)) {
		return false;
	}

	memcpy(&hdr, bytes.data(), sizeof(ActListHeader));
	if(hdr.magic[0] != 'E' || hdr.magic[1] != 'A') {
		return false;
	}

	m_history = hdr.history;

	std::vector<L2dLump> layer_strings;

	if(!ReadLump(m_rects, hdr.rects, bytes.data())) return false;
	if(!ReadLump(m_lines, hdr.lines, bytes.data())) return false;
	if(!ReadLump(m_moves, hdr.moves, bytes.data())) return false;
	if(!ReadLump(m_scales, hdr.scales, bytes.data())) return false;
	if(!ReadLump(m_textures, hdr.textures, bytes.data())) return false;
	if(!ReadLump(m_indices, hdr.indices, bytes.data())) return false;
	
	return true;
}


void ActList::Seralize(std::vector<uint8_t> &bytes,
                       std::vector<uint8_t> &strings) const
{
	ActListHeader hdr;
	hdr.magic[0] = 'E';
	hdr.magic[1] = 'A';
	hdr.history = m_history;

	hdr.rects.ofs = sizeof(ActListHeader);
	hdr.rects.size = m_rects.size() * sizeof(ActRect);

	hdr.lines.ofs = hdr.rects.ofs + hdr.rects.size;
	hdr.lines.size = m_lines.size() * sizeof(ActLine);

	hdr.moves.ofs = hdr.lines.ofs + hdr.lines.size;
	hdr.moves.size = m_moves.size() * sizeof(ActMove);

	hdr.scales.ofs = hdr.moves.ofs + hdr.moves.size;
	hdr.scales.size = m_scales.size() * sizeof(ActScale);

	hdr.textures.ofs = hdr.scales.ofs + hdr.scales.size;
	hdr.textures.size = m_textures.size() * sizeof(ActTexture);

	hdr.indices.ofs = hdr.textures.ofs + hdr.textures.size;
	hdr.indices.size = m_indices.size() * sizeof(ActIndex);

	if(bytes.size() < BinSize()) {
		bytes.resize(BinSize());
	}

	memcpy(bytes.data(), &hdr, sizeof(ActListHeader));
	memcpy(bytes.data() + hdr.rects.ofs, m_rects.data(), hdr.rects.size);
	memcpy(bytes.data() + hdr.lines.ofs, m_lines.data(), hdr.lines.size);
	memcpy(bytes.data() + hdr.moves.ofs, m_moves.data(), hdr.moves.size);
	memcpy(bytes.data() + hdr.scales.ofs, m_scales.data(), hdr.scales.size);
	memcpy(bytes.data() + hdr.textures.ofs, m_textures.data(), hdr.textures.size);
	memcpy(bytes.data() + hdr.indices.ofs, m_indices.data(), hdr.indices.size);

	wxASSERT(hdr.indices.ofs + hdr.indices.size <= bytes.size());
}
