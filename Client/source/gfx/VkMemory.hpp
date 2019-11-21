#pragma once
#include <vulkan/vulkan_core.h>
#include "Defines.hpp"
#include "log/Logger.hpp"
#include <set>
#include <tuple>

struct MemorySegment
{
	uint32 beginIndex;
	uint32 endIndex;
	size_t realSize;
	void* metadata = 0;

	bool operator<(const MemorySegment& segment) const
	{
		return endIndex < segment.beginIndex;
	};

	bool operator==(const MemorySegment& segment) const
	{
		return beginIndex == segment.beginIndex && endIndex == segment.endIndex;
	};
};

struct BufferView
{
	uint32 id;
	VkBuffer buffer;
	VkDeviceMemory memory;
	size_t size;
	bool readonly;

	bool operator<(const BufferView& view) const
	{
		return id < view.id;
	};

	bool operator==(const BufferView& view) const
	{
		return id == view.id;
	};
};

struct BufferMeta
{
	std::set<MemorySegment> allocations;
	std::set<MemorySegment> freeSlots;
};

class VkMemory
{
public:
	size_t blockSize = 4096;
	size_t preallocationSize = 0;
	std::map<BufferView, BufferMeta> buffers;

	VkMemory()
	{
		preallocationSize = (size_t)pow(6.4f, 7.0f); // 64mb
	};

	uint32 getNextIndex(BufferView& view, size_t size)
	{
		auto freeSlots = buffers[view].freeSlots;
		if (!freeSlots.empty())
		{
			for (auto& seg : freeSlots)
			{
				if (seg.realSize >= size)
				{
					uint32 freeBlock = seg.beginIndex;
					uint32 nextSeg = getEndingSegment(size, freeBlock) + 1;
					if (nextSeg <= seg.endIndex)
					{
						auto copy = seg;
						freeSlots.erase(seg);
						copy.beginIndex = nextSeg;
						freeSlots.insert(copy);
					}
					else
					{
						freeSlots.erase(seg);
					}
					return freeBlock;
				}
			}
		}
		auto allocations = buffers[view].allocations;
		return (uint32)allocations.empty() ? 0 : (*allocations.rbegin()).endIndex + 1;
	};

	uint32 getEndingSegment(size_t size, uint32 begin)
	{
		return begin + (uint32)(size > blockSize ? (size / blockSize) : 0);
	};

	BufferView& makeBuffer(bool readonly, size_t minSize, bool preallocate = false)
	{
		uint32 index = (uint32)buffers.size();
		BufferView view;
		view.id = index;
	};

	void cleanup()
	{

	};

	/*void test()
	{
		MemorySegment seg1;
		seg1.realSize = (sizeof(float) * 3) * 128;
		seg1.beginIndex = getNextIndex(seg1.realSize);
		seg1.endIndex = seg1.beginIndex + (seg1.realSize > blockSize ? (seg1.realSize / blockSize) : 0);
		allocations.insert(seg1);
		debug("Segment 1: size: %d, starting block: %d, ending block: %d", seg1.realSize, seg1.beginIndex, seg1.endIndex);

		MemorySegment seg2;
		seg2.realSize = (sizeof(float) * 3) * 128;
		seg2.beginIndex = getNextIndex(seg2.realSize);
		seg2.endIndex = seg2.beginIndex + (seg2.realSize > blockSize ? (seg2.realSize / blockSize) : 0);
		allocations.insert(seg2);
		debug("Segment 2: size: %d, starting block: %d, ending block: %d", seg2.realSize, seg2.beginIndex, seg2.endIndex);

		MemorySegment seg3;
		seg3.realSize = (sizeof(float) * 3) * 512;
		seg3.beginIndex = getNextIndex(seg3.realSize);
		seg3.endIndex = seg3.beginIndex + (seg3.realSize > blockSize ? (seg3.realSize / blockSize) : 0);
		allocations.insert(seg3);
		debug("Segment 3: size: %d, starting block: %d, ending block: %d", seg3.realSize, seg3.beginIndex, seg3.endIndex);

		MemorySegment seg4;
		seg4.realSize = (sizeof(float) * 3) * 128;
		seg4.beginIndex = getNextIndex(seg4.realSize);
		seg4.endIndex = seg4.beginIndex + (seg4.realSize > blockSize ? (seg4.realSize / blockSize) : 0);
		allocations.insert(seg4);
		debug("Segment 4: size: %d, starting block: %d, ending block: %d", seg4.realSize, seg4.beginIndex, seg4.endIndex);

		MemorySegment seg5;
		seg5.realSize = (sizeof(float) * 3) * 512;
		seg5.beginIndex = getNextIndex(seg5.realSize);
		seg5.endIndex = seg5.beginIndex + (seg5.realSize > blockSize ? (seg5.realSize / blockSize) : 0);
		allocations.insert(seg5);
		debug("Segment 5: size: %d, starting block: %d, ending block: %d", seg5.realSize, seg5.beginIndex, seg5.endIndex);

		freeSlots.insert(seg2);
		allocations.erase(seg2);
		MemorySegment seg6;
		seg6.realSize = (sizeof(float) * 3) * 128;
		seg6.beginIndex = getNextIndex(seg6.realSize);
		seg6.endIndex = seg6.beginIndex + (seg6.realSize > blockSize ? (seg6.realSize / blockSize) : 0);
		allocations.insert(seg6);
		debug("Segment 6: size: %d, starting block: %d, ending block: %d", seg6.realSize, seg6.beginIndex, seg6.endIndex);

		freeSlots.insert(seg6);
		allocations.erase(seg6);
		seg6.realSize = (sizeof(float) * 3) * 512;
		seg6.beginIndex = getNextIndex(seg6.realSize);
		seg6.endIndex = seg6.beginIndex + (seg6.realSize > blockSize ? (seg6.realSize / blockSize) : 0);
		allocations.insert(seg6);
		debug("Segment 6: size: %d, starting block: %d, ending block: %d", seg6.realSize, seg6.beginIndex, seg6.endIndex);

		MemorySegment seg7;
		seg7.realSize = (sizeof(float) * 3) * 128;
		seg7.beginIndex = getNextIndex(seg7.realSize);
		seg7.endIndex = seg7.beginIndex + (seg7.realSize > blockSize ? (seg7.realSize / blockSize) : 0);
		allocations.insert(seg7);
		debug("Segment 7: size: %d, starting block: %d, ending block: %d", seg7.realSize, seg7.beginIndex, seg7.endIndex);

		freeSlots.insert(seg3);
		allocations.erase(seg3);
		MemorySegment seg8;
		seg8.realSize = (sizeof(float) * 3) * 128;
		seg8.beginIndex = getNextIndex(seg8.realSize);
		seg8.endIndex = seg8.beginIndex + (seg8.realSize > blockSize ? (seg8.realSize / blockSize) : 0);
		allocations.insert(seg8);
		debug("Segment 8: size: %d, starting block: %d, ending block: %d", seg8.realSize, seg8.beginIndex, seg8.endIndex);

		MemorySegment seg9;
		seg9.realSize = (sizeof(float) * 3) * 128;
		seg9.beginIndex = getNextIndex(seg9.realSize);
		seg9.endIndex = seg9.beginIndex + (seg9.realSize > blockSize ? (seg9.realSize / blockSize) : 0);
		allocations.insert(seg9);
		debug("Segment 9: size: %d, starting block: %d, ending block: %d", seg9.realSize, seg9.beginIndex, seg9.endIndex);

		MemorySegment seg10;
		seg10.realSize = (sizeof(float) * 3) * 128;
		seg10.beginIndex = getNextIndex(seg10.realSize);
		seg10.endIndex = seg10.beginIndex + (seg10.realSize > blockSize ? (seg10.realSize / blockSize) : 0);
		allocations.insert(seg10);
		debug("Segment 10: size: %d, starting block: %d, ending block: %d", seg10.realSize, seg10.beginIndex, seg10.endIndex);
	}*/
};
