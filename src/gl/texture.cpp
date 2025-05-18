#include <cstring>

#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include "src/edit/editorcontext.hpp"
#include "src/gl/texture.hpp"


static uint32_t fnv1a(unsigned char *data, size_t size)
{
	static constexpr uint32_t OFFSET_BASIS = 0x811C9DC5;
	static constexpr uint32_t PRIME = 0x01000193;

	size_t hash = OFFSET_BASIS;

	for(size_t i = 0; i < size; i++) {
		hash ^= data[i];
		hash *= PRIME;
	}

	return hash;
}


static int bitceil512(int n)
{
	n = std::min(n, 512);
	int i = 1;
	while(i < n) {
		i <<= 1;
	}
	return i;
}


void gl::texture::serialize(l2d::texinfo &info, std::vector<unsigned char> &data, std::vector<unsigned char> &strings) const
{
	info.name_ofs = strings.size();
	info.name_size = m_name.size();
	strings.insert(strings.end(), m_name.begin(), m_name.end());

	info.width = m_width;
	info.height = m_height;
	info.pixelwidth = m_pixelwidth;

	info.data_ofs = data.size();
	size_t nbytes = m_width * m_height * m_pixelwidth;
	data.resize(data.size() + nbytes);
	memcpy(data.data() + info.data_ofs, m_data, nbytes);

}

void gl::texture::load(size_t width, size_t height, size_t pixelwidth, const char *name, unsigned char *data)
{
	m_name = name;
	m_width = width;
	m_height = height;
	m_pixelwidth = pixelwidth;
	m_hash = fnv1a(m_data, width * height * pixelwidth);
}


bool gl::texture::load(const char *path)
{
	int w, h, nchan;
	unsigned char *data = stbi_load(path, &w, &h, &nchan, 0);

	if(data == nullptr) {
		return false;
	}

	size_t width  = bitceil512(w);
	size_t height = bitceil512(h);

	unsigned char *new_data = new unsigned char[width * height * nchan];

	m_data = stbir_resize_uint8_linear(data, w, h, w * nchan, 
		new_data, width, height, m_width * nchan, (stbir_pixel_layout)nchan);

	assert(m_data == new_data);

	load(width, height, nchan, path, m_data);

	stbi_image_free(data);
}


void gl::texture::init_gltex()
{
	glGenTextures(1, &m_gltex);
	glBindTexture(GL_TEXTURE_2D, m_gltex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(m_pixelwidth == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, m_data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, m_data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}


bool gl::texture::operator==(const texture &other)
{
	// broad phase
	if(m_hash != other.m_hash) {
		return false;
	}

	size_t nbytes = m_width * m_height * m_pixelwidth;

	if(nbytes != other.m_width * other.m_height * other.m_pixelwidth) {
		return false;
	}

	// narrow phase
	return memcmp(m_data, other.m_data, nbytes) == 0;
}


void gl::texture::free()
{
	if(glIsTexture(m_gltex)) {
		glDeleteTextures(1, &m_gltex);
	}

	if(m_data != nullptr) {
		delete[] m_data;
	}
}
