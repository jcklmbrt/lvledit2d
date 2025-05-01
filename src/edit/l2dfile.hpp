
#ifndef _L2DFILE_HPP
#define _L2DFILE_HPP

#include <vector>
#include <cstdint>

struct L2dLump
{
	uint32_t ofs, size;
};

/* stripped down version of GLTexture */
struct L2dTexInfo
{
	uint32_t name_ofs;
	uint32_t name_size;
	uint32_t width;
	uint32_t height;
	uint8_t  pixelwidth;
	uint32_t data_ofs;

	inline uint32_t GetSize() const
	{
		return width * height * pixelwidth;
	}
};


class EditorContext;

class L2dFile
{
public:
	bool LoadFromFile(const char *filename);
	bool WriteToFile(const char *filename) const;
	bool LoadFromContext(const EditorContext *edit);
	bool WriteToContext(EditorContext *edit) const;
private:
	std::vector<L2dTexInfo> m_texinfo;
	std::vector<uint8_t> m_actiondata;
	std::vector<uint8_t> m_texdata;
	std::vector<uint8_t> m_strings;
};

#endif
