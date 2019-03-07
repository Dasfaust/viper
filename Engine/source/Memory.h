#pragma once
#include <unordered_map>
#include <tuple>
#include "Logger.h"

namespace Memory
{
	class ObjectBase
	{
	public:
		unsigned int heapSection;
		unsigned int id;
		unsigned int iid;
		unsigned int parent;

		virtual ~ObjectBase()
		{
		
		}
	};

	template<typename T>
	class HeapObject : public ObjectBase
	{
	public:
		//static const HeapObject<T>* static_instance;
		static const size_t size;
	};

	/*template<typename T>
	const HeapObject<T>* HeapObject<T>::static_instance = new HeapObject<T>();*/
	template<typename T>
	const size_t HeapObject<T>::size = sizeof(T);

	static std::unordered_map<unsigned int, std::vector<ObjectBase*>*> objects;
	static std::vector<std::vector<ObjectBase*>> heapPointers;
	static std::vector<std::vector<std::byte>> heap;

	inline unsigned int reserve()
	{
		/*unsigned int id = objects.size();
		auto memory = new std::vector<ObjectBase*>();
		objects[id] = memory;
		return id;*/
		unsigned int id = heapPointers.size();
		heapPointers.resize(id + 1);
		heap.resize(id + 1);
		heapPointers[id] = std::vector<ObjectBase*>();
		heap[id] = std::vector<std::byte>();
		return id;
	};

	template<typename T>
	inline unsigned int create(unsigned int heapSection)
	{
		/*unsigned int id = objects[heapSection]->size();
		objects[heapSection]->emplace_back(new T());
		(*objects[heapSection])[id]->id = id;
		return id;*/
		unsigned int id = heapPointers[heapSection].size();
		heapPointers[heapSection].resize(id + 1);
		unsigned int iid = heap[heapSection].size();
		heap[heapSection].resize(iid + T::size);
		T* ob = new(&heap[heapSection][iid]) T();
		heapPointers[heapSection][id] = ob;
		ob->heapSection = heapSection;
		ob->id = id;
		ob->iid = iid;
		return id;
	};

	template<typename T>
	inline T* get(unsigned int heapSection, unsigned int id)
	{
		//return static_cast<T*>((*objects[heapSection])[id]);
		return static_cast<T*>(heapPointers[heapSection][id]);
	};

	inline void erase(unsigned int heapSection, unsigned int id)
	{
		delete (*objects[heapSection])[id];
		objects[heapSection]->erase(objects[heapSection]->begin() + id);
	};

	inline void purge()
	{
		for (auto &kv : objects)
		{
			auto arr = kv.second;
			for (int i = 0; i < arr->size(); i++)
			{
				delete (*arr)[i];
			}
			delete kv.second;
		}
		objects.clear();
	};

	class SomeObject : public HeapObject<SomeObject>
	{
	public:
		int someInt = 24;
	};

	class OtherObject : public HeapObject<SomeObject>
	{
	public:
		std::string str = "hello, world!";
	};

	inline void test()
	{
		/*unsigned int heapSection = reserve();

		auto ob1 = get<SomeObject>(heapSection, create<SomeObject>(heapSection));

		debugf("Memory: object id %d", ob1->id);
		debugf("Memory: object val %d", ob1->someInt);

		//erase(heapSection, ob1->id);

		auto ob2 = get<OtherObject>(heapSection, create<OtherObject>(heapSection));

		debugf("Memory: object id %d", ob2->id);
		debugf("Memory: object val %s", ob2->str.c_str());*/
	};
};