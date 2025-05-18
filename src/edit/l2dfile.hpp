
#ifndef _L2DFILE_HPP
#define _L2DFILE_HPP

#include <vector>
#include <cstdint>

namespace l2d {
class editor;
/* stripped down version of GLTexture */
struct texinfo {
	uint32_t name_ofs;
	uint32_t name_size;
	uint32_t width;
	uint32_t height;
	uint8_t  pixelwidth;
	uint32_t data_ofs;
	inline uint32_t size() const
	{
		return width * height * pixelwidth;
	}
};
struct file {
	bool load(const char *filename);
	bool save(const char *filename) const;
	bool load(const l2d::editor &edit);
	bool save(l2d::editor &edit) const;
private:
	std::vector<texinfo> m_texinfo;
	std::vector<uint8_t> m_actiondata;
	std::vector<uint8_t> m_texdata;
	std::vector<uint8_t> m_strings;
};
}

#endif
