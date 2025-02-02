#include <wx/bitmap.h>
#include <wx/filename.h>
#include "src/gl/texture.hpp"
#include "src/texturepanel.hpp"

static int bit_ceil_512(int n)
{
	n = std::min(n, 512);
	int i = 1;
	while(i < n) {
		i <<= 1;
	}
	return i;
}


Texture::Texture(const wxFileName &filename)
	: m_filename(filename)
{
	wxASSERT(filename.IsOk() && filename.Exists());

	wxImage img(filename.GetFullPath());

	wxASSERT(img.IsOk());

	if(img.HasAlpha()) {
		m_pixelwidth = 4;
	}
	else {
		m_pixelwidth = 3;
	}

	wxBitmap bmp(img);

	TextureList *tlist = TextureList::GetInstance();
	wxImageList *ilist = tlist->GetImageList(wxIMAGE_LIST_SMALL);

	wxSize size = img.GetSize();
	size.y = bit_ceil_512(size.y);
	size.x = bit_ceil_512(size.x);

	/* bananas */
	wxBitmap::Rescale(bmp, size);
	img = bmp.ConvertToImage();

	wxBitmap::Rescale(bmp, wxSize(THUMB_SIZE_X, THUMB_SIZE_Y));
	m_index = ilist->Add(bmp);

	m_width = img.GetWidth();
	m_height = img.GetHeight();
	size_t npix = m_width * m_height;

	unsigned char *alpha = img.GetAlpha();
	unsigned char *data = img.GetData();

	m_data = new unsigned char[npix * m_pixelwidth];

	for(size_t i = 0; i < npix; i++) {
		m_data[i * m_pixelwidth + 0] = data[i * 3 + 0];
		m_data[i * m_pixelwidth + 1] = data[i * 3 + 1];
		m_data[i * m_pixelwidth + 2] = data[i * 3 + 2];
		if(m_pixelwidth == 4) {
			m_data[i * 4 + 3] = alpha[i];
		}
	}

	m_hash = FNV1A();
}


void Texture::InitTextureObject()
{
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(m_pixelwidth == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, m_data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, m_data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}


uint32_t Texture::FNV1A()
{
	constexpr uint32_t FNV_OFFSET_BASIS = 0x811c9dc5;
	constexpr uint32_t FNV_PRIME = 0x01000193;

	size_t nbytes = m_width * m_height * m_pixelwidth;
	size_t hash = FNV_OFFSET_BASIS;

	for(size_t i = 0; i < nbytes; i++) {
		hash ^= m_data[i];
		hash *= FNV_PRIME;
	}

	return hash;
}


bool Texture::operator==(const Texture &other)
{
	/* broad phase */
	if(m_hash != other.m_hash
		|| m_width != other.m_width
		|| m_height != other.m_height
		|| m_pixelwidth != other.m_pixelwidth) {
		return false;
	}

	/* narrow phase */
	size_t nbytes = m_width * m_height * m_pixelwidth;
	return memcmp(m_data, other.m_data, nbytes) == 0;
}


Texture::Texture(Texture &&other) noexcept
{
	/* default copy */
	this->operator=(other);

	other.m_data = nullptr;
	other.m_texture = 0;
	other.m_filename.Clear();
}


Texture::~Texture()
{
	if(glIsTexture(m_texture)) {
		glDeleteTextures(1, &m_texture);
	}

	if(m_data != nullptr) {
		delete[] m_data;
	}
}


GLuint Texture::GetTextureObject()
{
	if(!glIsTexture(m_texture)) {
		InitTextureObject();
	}

	return m_texture;
}