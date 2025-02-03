#include <cstring>
#include <wx/bitmap.h>
#include <wx/filename.h>
#include <wx/gtk/bitmap.h>
#include <wx/image.h>
#include "src/edit/editorcontext.hpp"
#include "src/gl/texture.hpp"
#include "src/texturepanel.hpp"


#define FNV_OFFSET_BASIS 0x811C9DC5
#define FNV_PRIME 0x01000193


static uint32_t FNV1A(unsigned char *Data, size_t Size)
{
	size_t Hash = FNV_OFFSET_BASIS;

	for(size_t i = 0; i < Size; i++) {
		Hash ^= Data[i];
		Hash *= FNV_PRIME;
	}

	return Hash;
}

static int BitCeil512(int N)
{
	N = std::min(N, 512);
	int i = 1;
	while(i < N) {
		i <<= 1;
	}
	return i;
}

void LoadTextureFromMemory(GLTexture *Texture, size_t Width, size_t Height, size_t PixelWidth, const char *Name, unsigned char *Data)
{
	memset(Texture->Name, 0, sizeof(Texture->Name));
	strncpy(Texture->Name, Name, sizeof(Texture->Name));

	wxImage img(Width, Height, Data);
	wxBitmap bmp(img);
	TextureList *tlist = TextureList::GetInstance();
	wxImageList *ilist = tlist->GetImageList(wxIMAGE_LIST_SMALL);
	wxBitmap::Rescale(bmp, wxSize(THUMB_SIZE_X, THUMB_SIZE_Y));
	Texture->ThumbIndex = ilist->Add(bmp);

	size_t NumBytes = Width * Height * PixelWidth;

	Texture->Width = Width;
	Texture->Height = Height;
	Texture->PixelWidth = PixelWidth;
	Texture->Data = Data;
	Texture->HashValue = FNV1A(Texture->Data, NumBytes);
}

void LoadTextureFromFile(GLTexture *Texture, const wxFileName &FileName)
{
	wxASSERT(FileName.IsOk() && FileName.Exists());

	wxImage img(FileName.GetFullPath());

	wxASSERT(img.IsOk());

	size_t PixelWidth = 3;
	if(img.HasAlpha()) {
		PixelWidth = 4;
	}

	wxSize size = img.GetSize();
	size.y = BitCeil512(size.y);
	size.x = BitCeil512(size.x);
	img = img.Rescale(size.x, size.y);

	size_t Width = img.GetWidth();
	size_t Height = img.GetHeight();
	size_t NumPixels = Width * Height;

	unsigned char *A = img.GetAlpha();
	unsigned char *RGB = img.GetData();
	unsigned char *Data = new unsigned char[NumPixels * PixelWidth];
	const char *Name = FileName.GetName().c_str();

	for(size_t i = 0; i < NumPixels; i++) {
		Data[i * PixelWidth + 0] = RGB[i * 3 + 0];
		Data[i * PixelWidth + 1] = RGB[i * 3 + 1];
		Data[i * PixelWidth + 2] = RGB[i * 3 + 2];
		if(PixelWidth == 4) {
			Data[i * 4 + 3] = A[i];
		}
	}

	LoadTextureFromMemory(Texture, Width, Height, PixelWidth, Name, Data);
}


void InitTextureObject(GLTexture *Texture)
{
	glGenTextures(1, &Texture->TextureObject);
	glBindTexture(GL_TEXTURE_2D, Texture->TextureObject);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(Texture->PixelWidth == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture->Width, Texture->Height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, Texture->Data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture->Width, Texture->Height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, Texture->Data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool EqualTextures(GLTexture *A, GLTexture *B)
{
	/* broad phase */
	if(A->HashValue != B->HashValue) {
		return false;
	}

	size_t A_NumBytes = A->Width * A->Height * A->PixelWidth;
	size_t B_NumBytes = B->Width * B->Height * B->PixelWidth;

	if(A_NumBytes != B_NumBytes) {
		return false;
	}

	/* narrow phase */
	return memcmp(A->Data, B->Data, A_NumBytes) == 0;
}


void DeleteTexture(GLTexture *Texture)
{
	if(glIsTexture(Texture->TextureObject)) {
		glDeleteTextures(1, &Texture->TextureObject);
	}

	if(Texture->Data != nullptr) {
		delete[] Texture->Data;
	}
}
