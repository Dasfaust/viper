#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <cassert>
#include "../Logger.h"
#include "../util/Time.h"

struct t_info
{
	unsigned int id;
	size_t size;
	std::string name;
};

struct object
{
	unsigned int id;
	t_info* type;
};

class Pool
{
public:
	std::unordered_map<std::type_index, t_info*> types = std::unordered_map<std::type_index, t_info*>();
	std::unordered_map<unsigned int, std::vector<unsigned int>> heaps = std::unordered_map<unsigned int, std::vector<unsigned int>>();
	std::unordered_map<unsigned int, std::vector<unsigned int>> freeSlots = std::unordered_map<unsigned int, std::vector<unsigned int>>();

	template<typename T>
	inline t_info* resolve()
	{
		std::type_index index = std::type_index(typeid(T));
		if (types.count(index))
		{
			return types[index];
		}
		unsigned int id = types.size();
		t_info* type = new t_info();
		type->id = id;
		type->size = sizeof(T);
		type->name = typeid(T).name();
		types[index] = type;

		unsigned int obId = allocate(type);
		T* instance = new(&heaps[type->id][obId]) T();
		instance->id = obId;
		instance->type = types[index];

		return types[index];
	};

	inline unsigned int allocate(t_info* type)
	{
		unsigned int free;
		if (freeSlots.count(type->id) && !freeSlots[type->id].empty())
		{
			free = freeSlots[type->id][0];
			freeSlots[type->id].erase(freeSlots[type->id].begin());
		}
		else
		{
			free = heaps[type->id].size();
			heaps[type->id].resize(free + type->size);
		}
		return free;
	};

	inline unsigned int instantiate(t_info* type)
	{
		unsigned int id = allocate(type);
		auto instance = new(&heaps[type->id][id]) object(*reinterpret_cast<object*>(&heaps[type->id][0]));
		instance->id = id;
		return id;
	};

	inline object* get(t_info* type, unsigned int id)
	{
		return reinterpret_cast<object*>(&heaps[type->id][id]);
	};

	template<typename T>
	inline T* get(unsigned int id)
	{
		return reinterpret_cast<T*>(&heaps[resolve<T>()->id][id]);
	};

	inline object* create(t_info* type)
	{
		return get(type, instantiate(type));
	};

	template<typename T>
	inline T* create()
	{
		return reinterpret_cast<T*>(create(resolve<T>()));
	};

	inline void del(t_info* type, unsigned int id)
	{
		freeSlots[type->id].push_back(id);
	};

	template<typename T>
	inline void del(unsigned int id)
	{
		freeSlots[resolve<T>()->id].push_back(id);
	};

	inline std::vector<object*> getAll(t_info* type)
	{
		std::vector<object*> vec;
		for (unsigned int i = type->size; i < heaps[type->id].size(); i += type->size)
		{
			vec.push_back(get(type, i));
		}
		return vec;
	};

	template<typename T>
	inline std::vector<T*> getAll()
	{
		auto type = resolve<T>();
		std::vector<T*> vec;
		for (unsigned int i = type->size; i < heaps[type->id].size(); i += type->size)
		{
			vec.push_back(reinterpret_cast<T*>(get(type, i)));
		}
		return vec;
	};

	inline void purge()
	{
		for (auto&& kv : heaps)
		{
			kv.second.clear();
			kv.second.shrink_to_fit();
		}
		heaps.clear();
		freeSlots.clear();
		for (auto&& kv : types)
		{
			delete kv.second;
		}
		heaps.clear();
	};

	struct TestObject : public object
	{
		bool testBool;
		int testInt;
	};

	inline void test()
	{
		double t = tnowns();
		TestObject* ob1 = new TestObject();
		debugf("New: %.2f", tnowns() - t);
		delete ob1;

		t = tnowns();
		auto ob2 = std::make_shared<TestObject>();
		debugf("Shared: %.2f", tnowns() - t);

		t = tnowns();
		std::vector<char> vec;
		vec.resize(sizeof(TestObject));
		object* ob3 = new(&vec[0]) TestObject();
		debugf("Static: %.2f", tnowns() - t);
		vec.clear();
		vec.shrink_to_fit();

		debug("### Memory pool test");
		TestObject* test = create<TestObject>();
		test->testBool = false;
		test->testInt = 420;

		auto type = resolve<TestObject>();
		assert(type->name == std::string(typeid(TestObject).name()));
		assert(get<TestObject>(test->id)->type->id == type->id);
		assert(get<TestObject>(test->id)->type->size == type->size);
		assert(get<TestObject>(test->id)->testBool == false);
		assert(get<TestObject>(test->id)->testInt == 420);
		debug("### Templated creation is successful");

		unsigned int deleted = test->id;
		del<TestObject>(deleted);

		unsigned int id = instantiate(type);
		auto ob = get(type, id);
		reinterpret_cast<TestObject*>(ob)->testBool = false;
		reinterpret_cast<TestObject*>(ob)->testInt = 420;

		assert(reinterpret_cast<TestObject*>(ob)->type->id == type->id);
		assert(reinterpret_cast<TestObject*>(ob)->type->size == type->size);
		assert(reinterpret_cast<TestObject*>(ob)->testBool == false);
		assert(reinterpret_cast<TestObject*>(ob)->testInt == 420);
		debug("### Untemplated creation is successful");

		assert(reinterpret_cast<TestObject*>(ob)->id == deleted);
		debug("### Deletion successful");

		double start = tnow();
		for (int i = 1; i < 1000000; i++)
		{
			create<TestObject>();
		}
		debugf("### Created 1M templated objects in %.2fms", tnow() - start);

		start = tnow();
		assert(getAll<TestObject>().size() == 1000000);
		debugf("### Templated get all successful, get in %.2fms", tnow() - start);

		start = tnow();
		assert(getAll(type).size() == 1000000);
		debugf("### Untemplated get all successful, get in %.2fms", tnow() - start);

		start = tnow();
		for (auto ob : getAll(type))
		{
			del(type, ob->id);
		}
		assert(create(type)->id == type->size);
		debugf("### Untemplated delete all successful in %.2fms", tnow() - start);
	};
};