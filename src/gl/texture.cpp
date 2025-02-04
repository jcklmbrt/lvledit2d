#include <cstring>
#include <wx/bitmap.h>
#include <wx/filename.h>
#include <wx/image.h>
#include "src/edit/editorcontext.hpp"
#include "src/gl/texture.hpp"
#include "src/texturepanel.hpp"


#define FNV_OFFSET_BASIS 0x811C9DC5
#define FNV_PRIME 0x01000193


static uint32_t FNV1A(unsigned char *data, size_t size)
{
	size_t hash = FNV_OFFSET_BASIS;

	for(size_t i = 0; i < size; i++) {
		hash ^= data[i];
		hash *= FNV_PRIME;
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

void GLTexture::Load(size_t width, size_t height, size_t pixelwidth, const char *name, unsigned char *data)
{
	strncpy(this->name, name, sizeof(this->name));

	wxImage img(width, height, data);
	wxBitmap bmp(img);
	TextureList *tlist = TextureList::GetInstance();
	wxImageList *ilist = tlist->GetImageList(wxIMAGE_LIST_SMALL);
	wxBitmap::Rescale(bmp, wxSize(THUMB_SIZE_X, THUMB_SIZE_Y));
	this->thumb = ilist->Add(bmp);

	size_t nbytes = width * height * pixelwidth;

	this->width = width;
	this->height = height;
	this->pixelwidth = pixelwidth;

	/* We have to force a copy here. ~wxBitMapData() will delete our data. WTF. */
	this->data = new unsigned char[nbytes];
	memcpy(this->data, data, nbytes);

	this->hash = FNV1A(this->data, nbytes);
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
		pixelwidth = 4;
	} else {
		pixelwidth = 3;
	}

	width = img.GetWidth();
	height = img.GetHeight();
	size_t npix = width * height;

	unsigned char *alpha = img.GetAlpha();
	unsigned char *rgb = img.GetData();

	data = new unsigned char[npix * pixelwidth];

	wxString name = filename.GetName();

	for(size_t i = 0; i < npix; i++) {
		data[i * pixelwidth + 0] = rgb[i * 3 + 0];
		data[i * pixelwidth + 1] = rgb[i * 3 + 1];
		data[i * pixelwidth + 2] = rgb[i * 3 + 2];
		if(pixelwidth == 4) {
			data[i * 4 + 3] = alpha[i];
		}
	}

	Load(width, height, pixelwidth, name.c_str(), data);
}


void GLTexture::InitTextureObject()
{
	glGenTextures(1, &gltex);
	glBindTexture(GL_TEXTURE_2D, gltex);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(pixelwidth == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}


bool GLTexture::operator==(const GLTexture &other)
{
	/* broad phase */
	if(hash != other.hash) {
		return false;
	}

	size_t nbytes = width * height * pixelwidth;

	if(nbytes != other.width * other.height * other.pixelwidth) {
		return false;
	}

	/* narrow phase */
	return memcmp(data, other.data, nbytes) == 0;
}


void GLTexture::Delete()
{
	if(glIsTexture(gltex)) {
		glDeleteTextures(1, &gltex);
	}

	if(data != nullptr) {
		delete[] data;
	}
}
