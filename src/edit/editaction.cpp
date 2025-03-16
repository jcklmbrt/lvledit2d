#include "src/edit/editaction.hpp"

struct Lump 
{
	uint32_t size, ofs;
};

struct ActListHeader
{
	uint8_t magic[2];
	uint32_t history;
	Lump rects;
	Lump lines;
	Lump moves;
	Lump scales;
	Lump textures;
	Lump indices;
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
	wxASSERT(!m_indices.empty());
	ActIndex idx = m_indices.back();
	wxASSERT(idx.type == act.type && idx.poly == act.poly && idx.layer == act.layer);
	switch(act.type) {
	case ActType::RECT: m_rects[idx.index] = act.rect; break;
	case ActType::LINE: m_lines[idx.index] = act.line; break;
	case ActType::MOVE: m_moves[idx.index] = act.move; break;
	case ActType::SCALE: m_scales[idx.index] = act.scale; break;
	case ActType::TEXTURE: m_textures[idx.index] = act.texture; break;
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

bool ActList::GetAction(size_t i, ActData &act)
{
	if(i < 0 || i >= m_indices.size()) {
		return false;
	}

	GetAction(m_indices[i], act);
	return true;
}

void ActList::GetAction(const ActIndex &idx, ActData &act)
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

size_t ActList::BinSize()

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
static bool ReadLump(std::vector<T> &vec, const Lump &lump, unsigned char *bytes)
{
	if(lump.size % sizeof(T) != 0) {
		return false;
	}

	vec.resize(lump.size / sizeof(T));
	memcpy(vec.data(), bytes + lump.ofs, lump.size);
	return true;
}


bool ActList::Deserialize(unsigned char *bytes, size_t nbytes)
{
	ActListHeader hdr;
	memcpy(&hdr, bytes, sizeof(ActListHeader));
	if(hdr.magic[0] != 'E' || hdr.magic[1] != 'A') {
		return false;
	}

	m_history = hdr.history;

	if(!ReadLump(m_rects, hdr.rects, bytes)) return false;
	if(!ReadLump(m_lines, hdr.lines, bytes)) return false;
	if(!ReadLump(m_moves, hdr.moves, bytes)) return false;
	if(!ReadLump(m_scales, hdr.scales, bytes)) return false;
	if(!ReadLump(m_textures, hdr.textures, bytes)) return false;
	if(!ReadLump(m_indices, hdr.indices, bytes)) return false;
	
	return true;
}

template<typename T>
static void WriteLump(const std::vector<T> &vec, Lump &lump, unsigned char *bytes, size_t &ofs)
{
	size_t nbytes = vec.size() * sizeof(T);
	lump.size = nbytes;
	lump.ofs = ofs;
	memcpy(bytes + ofs, vec.data(), nbytes);
	ofs += nbytes;
}


void ActList::Seralize(unsigned char *bytes, size_t nbytes)
{
	wxASSERT(nbytes >= BinSize());
	ActListHeader hdr;
	hdr.magic[0] = 'E';
	hdr.magic[1] = 'A';
	hdr.history = m_history;

	size_t ofs = 0;
	WriteLump(m_rects, hdr.rects, bytes, ofs);
	WriteLump(m_lines, hdr.lines, bytes, ofs);
	WriteLump(m_moves, hdr.moves, bytes, ofs);
	WriteLump(m_scales, hdr.scales, bytes, ofs);
	WriteLump(m_textures, hdr.textures, bytes, ofs);
	WriteLump(m_indices, hdr.indices, bytes, ofs);
	memcpy(bytes, &hdr, sizeof(ActListHeader));
}