#include <cstring>
#include <wx/bitmap.h>
#include <wx/filename.h>
#include <wx/image.h>
#include "src/edit/editorcontext.hpp"
#include "src/gl/texture.hpp"
#include "src/texturepanel.hpp"


static uint32_t FNV1A(unsigned char *data, size_t size)
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

static int BitCeil512(int n)
{
	n = std::min(n, 512);
	int i = 1;
	while(i < n) {
		i <<= 1;
	}
	return i;
}

void GLTexture::AddToFile(TexInfo &info, std::vector<unsigned char> &data)
{
	strncpy(info.name, m_name, MAX_TEXTURE_NAME);
	info.width = m_width;
	info.height = m_height;
	info.pixelwidth = m_pixelwidth;

	info.dataoffset = data.size();
	size_t nbytes = m_width * m_height * m_pixelwidth;
	data.resize(data.size() + nbytes);
	memcpy(data.data() + info.dataoffset, m_data, nbytes);

}

void GLTexture::Load(size_t width, size_t height, size_t pixelwidth, const char *name, unsigned char *data)
{
	strncpy(m_name, name, sizeof(m_name));

	wxImage img(width, height, data);
	wxBitmap bmp(img);
	TextureList *tlist = TextureList::GetInstance();
	wxImageList *ilist = tlist->GetImageList(wxIMAGE_LIST_SMALL);
	wxBitmap::Rescale(bmp, wxSize(THUMB_SIZE_X, THUMB_SIZE_Y));
	m_thumb = ilist->Add(bmp);

	size_t nbytes = width * height * pixelwidth;

	m_width = width;
	m_height = height;
	m_pixelwidth = pixelwidth;

	/* We have to force a copy here. ~wxBitMapData() will delete our data. WTF. */
	m_data = new unsigned char[nbytes];
	memcpy(m_data, data, nbytes);

	m_hash = FNV1A(m_data, nbytes);
}

void GLTexture::Load(const wxFileName &filename)
{
	wxASSERT(filename.IsOk() && filename.Exists());

	wxImage img(filename.GetFullPath());

	wxASSERT(img.IsOk());

	wxSize size = img.GetSize();
	size.x = BitCeil512(size.x);
	size.y = BitCeil512(size.y);
	img.Rescale(size.x, size.y);

	if(img.HasAlpha()) {
		m_pixelwidth = 4;
	} else {
		m_pixelwidth = 3;
	}

	m_width = img.GetWidth();
	m_height = img.GetHeight();
	size_t npix = m_width * m_height;

	unsigned char *alpha = img.GetAlpha();
	unsigned char *rgb = img.GetData();

	m_data = new unsigned char[npix * m_pixelwidth];

	wxString name = filename.GetName();

	for(size_t i = 0; i < npix; i++) {
		m_data[i * m_pixelwidth + 0] = rgb[i * 3 + 0];
		m_data[i * m_pixelwidth + 1] = rgb[i * 3 + 1];
		m_data[i * m_pixelwidth + 2] = rgb[i * 3 + 2];
		if(m_pixelwidth == 4) {
			m_data[i * 4 + 3] = alpha[i];
		}
	}

	Load(m_width, m_height, m_pixelwidth, name.c_str(), m_data);
}


void GLTexture::InitTextureObject()
{
	glGenTextures(1, &m_gltex);
	glBindTexture(GL_TEXTURE_2D, m_gltex);
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


bool GLTexture::operator==(const GLTexture &other)
{
	/* broad phase */
	if(m_hash != other.m_hash) {
		return false;
	}

	size_t nbytes = m_width * m_height * m_pixelwidth;

	if(nbytes != other.m_width * other.m_height * other.m_pixelwidth) {
		return false;
	}

	/* narrow phase */
	return memcmp(m_data, other.m_data, nbytes) == 0;
}


void GLTexture::Delete()
{
	if(glIsTexture(m_gltex)) {
		glDeleteTextures(1, &m_gltex);
	}

	if(m_data != nullptr) {
		delete[] m_data;
	}
}
