#include <cstring>
#include "src/edit/editorcontext.hpp"
#include "src/edit/l2dfile.hpp"

namespace l2d {
struct lump {
	uint32_t ofs, size;
};
struct header {
	uint8_t magic[2];
	lump actions;
	lump texinfo;
	lump texdata;
	lump strings;
};
}


bool l2d::file::load(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if(fp == nullptr) {
		return false;
	}
	
	l2d::header hdr;
	fseek(fp, 0, SEEK_SET);
	fread(&hdr, sizeof(header), 1, fp);

	if(hdr.magic[0] != 'L' && hdr.magic[1] != '2') {
		fclose(fp);
		return false;
	}
	
	m_actiondata.resize(hdr.actions.size);
	fseek(fp, hdr.actions.ofs, SEEK_SET);
	fread(m_actiondata.data(), hdr.actions.size, 1, fp);

	m_texinfo.resize(hdr.texinfo.size / sizeof(texinfo));
	fseek(fp, hdr.texinfo.ofs, SEEK_SET);
	fread(m_texinfo.data(), hdr.texinfo.size, 1, fp);

	m_texdata.resize(hdr.texdata.size);
	fseek(fp, hdr.texdata.ofs, SEEK_SET);
	fread(m_texdata.data(), hdr.texdata.size, 1, fp);

	m_strings.resize(hdr.strings.size);
	fseek(fp, hdr.strings.ofs, SEEK_SET);
	fread(m_strings.data(), hdr.strings.size, 1, fp);

	fclose(fp);
	return true;
}

bool l2d::file::save(const char *filename) const
{
	FILE *fp = fopen(filename, "wb");
	if(fp == nullptr) {
		return false;
	}

	header hdr;
	hdr.magic[0] = 'L';
	hdr.magic[1] = '2';

	hdr.actions.ofs = sizeof(header);
	hdr.actions.size = m_actiondata.size();

	hdr.texinfo.ofs = hdr.actions.ofs + hdr.actions.size;
	hdr.texinfo.size = m_texinfo.size() * sizeof(texinfo);

	hdr.texdata.ofs = hdr.texinfo.ofs + hdr.texinfo.size;
	hdr.texdata.size = m_texdata.size();

	hdr.strings.ofs = hdr.texdata.ofs + hdr.texdata.size;
	hdr.strings.size = m_strings.size();

	fseek(fp, 0, SEEK_SET);
	fwrite(&hdr, sizeof(header), 1, fp);

	fseek(fp, hdr.actions.ofs, SEEK_SET);
	fwrite(m_actiondata.data(), hdr.actions.size, 1, fp);

	fseek(fp, hdr.texinfo.ofs, SEEK_SET);
	fwrite(m_texinfo.data(), hdr.texinfo.size, 1, fp);

	fseek(fp, hdr.texdata.ofs, SEEK_SET);
	fwrite(m_texdata.data(), hdr.texdata.size, 1, fp);

	fseek(fp, hdr.strings.ofs, SEEK_SET);
	fwrite(m_strings.data(), hdr.strings.size, 1, fp);

	return true;
}

bool l2d::file::load(const l2d::editor &edit)
{
	/*
	m_texinfo.clear();
	m_texdata.clear();
	m_strings.clear();

	for(const gl::texture &texture : edit.textures()) {
		texinfo &info = m_texinfo.emplace_back();
		texture.serialize(info, m_texdata, m_strings);
	}

	//const ActList &actions = edit->GetActList();
	//m_actiondata.resize(actions.BinSize());
	//actions.Seralize(m_actiondata, m_strings);
	*/
	return true;
}


bool l2d::file::save(l2d::editor &edit) const
{
	/*
	std::vector<gl::texture> textures = edit.textures();

	for(const texinfo &info : m_texinfo) {
		size_t data_size = info.size();
		// ~wxBitMapData() will delete data in GLTexture::Load!!!
		uint8_t *data = new uint8_t[data_size];

		char *name = new char[info.name_size];

		memcpy(data, m_texdata.data() + info.data_ofs, data_size);
		memcpy(name, m_strings.data() + info.name_ofs, info.name_size);

		gl::texture texture;
		texture.load(info.width, info.height, info.pixelwidth, name, data);

		textures.push_back(texture);

		delete[] name;
	}

	//ActList &actions = edit->GetActList();
	//actions.Deserialize(m_actiondata, m_strings);

	*/
	return true;
}
