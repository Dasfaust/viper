#pragma once
#include "../interface/Texture.hpp"
#include "util/IO.hpp"
#include "glad/glad.h"

namespace gfx
{
	class Texture2DOpenGL : public Texture2D
	{
	public:
		uint32 id;
		uint32 width;
		uint32 height;

		void init(const std::string& fileName) override
		{
			stbi_set_flip_vertically_on_load(1);
			auto resource = readImage(fileName);
			if (resource.data == nullptr) { throw std::invalid_argument("Texture image not found"); }
			width = resource.width;
			height = resource.height;

			GLenum glFormat = 0, glDataFormat = 0;
			if (resource.channels == 4)
			{
				glFormat = GL_RGBA8;
				glDataFormat = GL_RGBA;
			}
			else if (resource.channels == 3)
			{
				glFormat = GL_RGB8;
				glDataFormat = GL_RGB;
			}

			glCreateTextures(GL_TEXTURE_2D, 1, &id);
			glTextureStorage2D(id, 1, glFormat, width, height);

			glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTextureSubImage2D(id, 0, 0, 0, width, height, glDataFormat, GL_UNSIGNED_BYTE, resource.data);

			stbi_image_free(resource.data);
		};

		uint32 getWidth() override
		{
			return width;
		};

		uint32 getHeight() override
		{
			return height;
		};

		void bind(uint32 slot) override
		{
			glBindTextureUnit(slot, id);
		};

		void cleanup() override
		{
			glDeleteTextures(1, &id);
		};
	};
};