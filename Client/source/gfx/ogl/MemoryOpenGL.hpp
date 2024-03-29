#pragma once
#include "../interface/Memory.hpp"
#include "glad/glad.h"

namespace gfx
{
	static GLenum DataTypeOpenGL[]
	{
		GL_BOOL,
		GL_INT,
		GL_INT,
		GL_INT,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT
	};

	class BufferViewOpenGL : public BufferView
	{
	public:
		uint32 vertexArray;
		uint32 vertexBuffer;
		uint32 verticesCount;
		uint32 indexBuffer;
		uint32 indicesCount;
		uint32 instanceBuffer;

		void bind() override
		{
			glBindVertexArray(vertexArray);
		};

		void unbind() override
		{
			glBindVertexArray(0);
		};
	};

	class MemoryOpenGL : public Memory
	{
	public:
		std::vector<std::shared_ptr<BufferViewOpenGL>> buffers;
		umap(std::string, uint32) bufferNameToId;

		std::shared_ptr<BufferView> requestBuffer(std::string name, std::vector<float>* vertices, std::vector<uint32>* indices, BufferAttributeList attributes, BufferAttributeList instanceAttributes) override
		{
			// todo deal with existing buffer

			auto buffer = std::make_shared<BufferViewOpenGL>();
			buffer->id = (uint32)buffers.size();
			buffers.resize(buffer->id + 1);

			buffer->verticesCount = (uint32)((vertices->size() * sizeof(float)) / attributes.stride);
			
			glGenVertexArrays(1, &buffer->vertexArray);
			glBindVertexArray(buffer->vertexArray);

			glGenBuffers(1, &buffer->vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer->vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices->size(), vertices->data(), GL_STATIC_DRAW);

			uint32 nextAttribId = 0;
			for (uint32 i = 0; i < attributes.data.size(); i++)
			{
				glEnableVertexAttribArray(nextAttribId);
				glVertexAttribPointer(nextAttribId, attributes.data[i].count, DataTypeOpenGL[attributes.data[i].type], attributes.data[i].normalized ? GL_TRUE : GL_FALSE, (GLsizei)attributes.stride, (void*)attributes.data[i].offset);
				glVertexAttribDivisor(nextAttribId, attributes.data[i].divisor);
				nextAttribId++;
			}

			glGenBuffers(1, &buffer->instanceBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer->instanceBuffer);

			for (uint32 i = 0; i < instanceAttributes.data.size(); i++)
			{
				if (instanceAttributes.data[i].type == gfx::Float4x4)
				{
					size_t offset = 0;
					for (uint32 j = 0; j < 4; j++)
					{
						glEnableVertexAttribArray(nextAttribId);
						glVertexAttribPointer(nextAttribId, 4, DataTypeOpenGL[instanceAttributes.data[i].type], instanceAttributes.data[i].normalized ? GL_TRUE : GL_FALSE, (GLsizei)instanceAttributes.stride, (void*)offset);
						glVertexAttribDivisor(nextAttribId, instanceAttributes.data[i].divisor);
						offset += sizeof(float) * 4;
						nextAttribId++;
					}
				}
				else
				{
					glEnableVertexAttribArray(nextAttribId);
					glVertexAttribPointer(nextAttribId, instanceAttributes.data[i].count, DataTypeOpenGL[instanceAttributes.data[i].type], instanceAttributes.data[i].normalized ? GL_TRUE : GL_FALSE, (GLsizei)instanceAttributes.stride, (void*)instanceAttributes.data[i].offset);
					glVertexAttribDivisor(nextAttribId, instanceAttributes.data[i].divisor);
				}
			}

			if (indices != nullptr)
			{
				glGenBuffers(1, &buffer->indexBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->indexBuffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * indices->size(), indices->data(), GL_STATIC_DRAW);
				buffer->indicesCount = (uint32)indices->size();
			}
			else
			{
				buffer->indexBuffer = 0;
				buffer->indicesCount = 0;
			}

			glBindVertexArray(0);

			buffers[buffer->id] = buffer;
			bufferNameToId[name] = buffer->id;
			return buffer;  
		};

		bool isLoaded(std::string name) override
		{
			return bufferNameToId.find(name) != bufferNameToId.end();
		}

		std::shared_ptr<BufferView> getBuffer(std::string name) override
		{
			return buffers[bufferNameToId[name]];
		};

		std::shared_ptr<BufferView> getBuffer(uint32 id) override
		{
			return buffers[id];
		};

		void cleanup() override
		{
			for (auto buff : buffers)
			{
				glDeleteVertexArrays(0, &buff->vertexArray);
				glDeleteBuffers(1, &buff->vertexBuffer);
				if (buff->indexBuffer > 0) { glDeleteBuffers(1, &buff->indexBuffer); }
				glDeleteBuffers(1, &buff->instanceBuffer);
			}
		};
	};
};