#pragma once
#include "Defines.hpp"

namespace gfx
{
	enum DataType
	{
		Bool = 0, Int = 1, Int2 = 2, Int3 = 3, Float = 4, Float2 = 5, Float3 = 6, Float4 = 7, Float4x4 = 8
	};
	static size_t DataTypeSize[] =
	{
		sizeof(bool),
		sizeof(int),
		sizeof(int) * 2,
		sizeof(int) * 3,
		sizeof(float),
		sizeof(float) * 2,
		sizeof(float) * 3,
		sizeof(float) * 4,
		sizeof(float) * 4 * 4
	};
	static uint32 DataTypeCount[] =
	{
		1,
		1,
		2,
		3,
		1,
		2,
		3,
		4,
		4 * 4
	};

	struct BufferAttribute
	{
		DataType type;
		std::string name;
		size_t size;
		uint32 count;
		size_t offset;
		bool normalized;
		uint32 divisor;

		BufferAttribute(DataType type, const std::string& name, bool normalized = false, uint32 divisor = 0)
			: type(type), name(name), size(DataTypeSize[type]), count(DataTypeCount[type]), offset(0), normalized(normalized), divisor(divisor)
		{
			
		};
	};

	class BufferAttributeList
	{
	public:
		BufferAttributeList() = default;

		std::vector<BufferAttribute> data;
		size_t stride;

		BufferAttributeList(const std::initializer_list<BufferAttribute>& attributes)
			: data(attributes)
		{
			size_t offset = 0;
			stride = 0;
			for (auto& attrib : data)
			{
				attrib.offset = offset;
				offset += attrib.size;
				stride += attrib.size;
			}
		};
	};

	class BufferView
	{
	public:
		BufferAttributeList attributes;

		virtual void bind() = 0;
		virtual void unbind() = 0;
	};

	class Memory
	{
	public:
		virtual std::shared_ptr<BufferView> requestBuffer(std::string name, std::vector<float>* vertices, std::vector<uint32>* indices, BufferAttributeList attributes, BufferAttributeList instanceAttributes = { }) = 0;
		virtual bool isLoaded(std::string name) = 0;
		virtual std::shared_ptr<BufferView> getBuffer(std::string name) = 0;
		virtual void cleanup() = 0;
	};
};