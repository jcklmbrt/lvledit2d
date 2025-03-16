#ifndef _EDITACTION_HPP
#define _EDITACTION_HPP

#include <vector>
#include <glm/glm.hpp>

#include "src/geometry.hpp"
#include "src/historylist.hpp"

enum class ActType
{
	LINE,
	RECT,
	MOVE,
	SCALE,
	DEL,
	TEXTURE
};

typedef Rect2D ActRect;
typedef Plane2D ActLine;
typedef glm::i32vec2 ActMove;

struct ActScale
{
	glm::i32vec2 origin;
	glm::i32vec2 numer;
	glm::i32vec2 denom;
};

struct ActTexture
{
	int32_t index;
	int32_t scale;
};

struct ActIndex
{
	ActType type;
	size_t layer;
	size_t poly;
	size_t index;
};

struct ActData
{
	ActType type;
	size_t layer;
	size_t poly;
	union {
		ActRect rect;
		ActLine line;
		ActMove move;
		ActScale scale;
		ActTexture texture;
	};
};


class ActList
{
public:
	ActList() = default;
	bool Undo(ActData &act);
	bool Redo(ActData &act);
	void ClearFuture();
	void PopBack();
	bool GetBack(ActData &act);
	void UpdateBack(const ActData &act);

	size_t BinSize();
	void Seralize(unsigned char *bytes, size_t nbytes);
	bool Deserialize(unsigned char *bytes, size_t nbytes);

	bool GetAction(size_t i, ActData &act);
	void GetAction(const ActIndex &idx, ActData &act);
	void AddAction(const ActRect &rect, size_t poly, size_t layer);
	void AddAction(const ActLine &line, size_t poly, size_t layer);
	void AddAction(const ActMove &move, size_t poly, size_t layer);
	void AddAction(const ActScale &scale, size_t poly, size_t layer);
	void AddAction(const ActTexture &texture, size_t poly, size_t layer);
	void AddDelete(size_t poly, size_t layer);
private:
	// action types
	std::vector<ActRect> m_rects;
	std::vector<ActLine> m_lines;
	std::vector<ActMove> m_moves;
	std::vector<ActScale> m_scales;
	std::vector<ActTexture> m_textures;
	// [0..history]    --> history
	// [history..size] --> future
	std::vector<ActIndex> m_indices;
	size_t m_history = 0;
public:
	inline size_t TotalActions() { return m_indices.size(); }
	inline size_t HistoryIndex() { return m_history; }
	inline bool IsEmpty() { return m_indices.empty(); }
	
	template<typename T>
	inline void RemoveAction(const ActIndex &idx, std::vector<T> &vec)
	{
		size_t old_back = vec.size() - 1;
		if(old_back == idx.index) {
			vec.pop_back();
		} else if(idx.index < old_back) {
			std::swap(vec[idx.index], vec.back());
			vec.pop_back();
			for(size_t i = 0; i < m_indices.size(); i++) {
				if(m_indices[i].index == old_back && m_indices[i].type == idx.type) {
					m_indices[i].index = idx.index;
				}
			}
		}

		HistoryList *hlist = HistoryList::GetInstance();
		hlist->SetItemCount(m_indices.size());
		hlist->Refresh();
	}

	template<typename T, ActType type>
	inline void AddAction(std::vector<T> &vec, const T &inst, size_t poly, size_t layer)
	{
		ClearFuture();

		ActIndex idx;
		idx.type = type;
		idx.poly = poly;
		idx.layer = layer;

		vec.push_back(inst);
		idx.index = vec.size() - 1;

		m_indices.push_back(idx);
		m_history = m_indices.size();

		HistoryList *hlist = HistoryList::GetInstance();
		hlist->SetItemCount(m_indices.size());
		hlist->Refresh();
	}
};


#endif