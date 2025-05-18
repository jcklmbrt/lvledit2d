#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <string>
#include <glad/gl.h>
#include "src/edit/l2dfile.hpp"

namespace gl {
struct texture {
	static constexpr int THUMB_SIZE_X = 32;
	static constexpr int THUMB_SIZE_Y = 32;
	void load(size_t width, size_t height, size_t pixelwidth, const char *name, unsigned char *data);
	bool load(const char *path);
	void free();
	void init_gltex();
	void serialize(l2d::texinfo &info, std::vector<unsigned char> &data, std::vector<unsigned char> &strings) const;
	bool operator==(const texture &other);
private:
	GLuint m_gltex = 0;
	unsigned char *m_data = nullptr;
	size_t m_pixelwidth = 0;
	size_t m_width = 0;
	size_t m_height = 0;
	std::string m_name;
	size_t m_thumb = 0;
	uint32_t m_hash = 0;
public:
	GLuint gltex() const { return m_gltex; }
	const std::string &name() const { return m_name; }
	size_t thumb() const { return m_thumb; }
	size_t width() const { return m_width; }
	size_t height() const { return m_height; }
};
}

#endif