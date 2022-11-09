#include <string>
#include <functional>
#include <tuple>
#include <map>

#include "AnyObject.h"

const int MaxObjectNum = 10;

class superObjectPool
{
	template <typename T,typename... Args>
	using Constructor = std::function<std::shared_ptr<T>(Args...)>;
public:
	superObjectPool() : needClear(false){}
	~superObjectPool() { needClear = true; }

	//默认创建多少个对象
	template<typename T, typename... Args>
	void Create(int num)
	{
		if (num <= 0 || num > MaxObjectNum)
			throw std::logic_error("object num errer");

		auto constructName = typeid(Constructor<T, Args...>).name();

		Constructor<T, Args...> f = [constructName, this](Args... args)
		{
			return createPtr<T>(std::string(constructName), args...);
		};

		m_map.emplace(typeid(T).name(), f);

		m_counter.emplace(constructName, num);
	}


	template<typename T, typename... Args>
	std::shared_ptr<T> createPtr(std::string/*&*/ constructName, Args... args)
	{
		return std::shared_ptr<T>(new T(args...), [constructName, this](T* t)
		{
			if (needClear)
				delete[] t;
			else
				m_object_map.emplace(constructName, std::shared_ptr<T>(t));
		});
	}

	template<typename T, typename... Args>
	std::shared_ptr<T> Get(Args... args)
	{
		using ConstructType = Constructor<T, Args...>;

		std::string constructName = typeid(ConstructType).name();
		auto range = m_map.equal_range(typeid(T).name());

		for (auto it = range.first; it != range.second; ++it)
		{
			if (it->second.Is<ConstructType>())
			{
				auto ptr = GetInstance<T>(constructName, args...); //这里返回前，erase掉元素

				if (ptr != nullptr)
					return ptr;

				return CreateInstance<T, Args...>(it->second, constructName, args...);//为空时，创建一个元素
			}
		}

		return nullptr;
	}
private:
	template<typename T, typename... Args>
	std::shared_ptr<T> CreateInstance(Any& any,
		std::string& constructName, Args... args)
	{
		using ConstructType = Constructor<T, Args...>;
		ConstructType f = any.AnyCast<ConstructType>();

		return createPtr<T, Args...>(constructName, args...);
	}

	template<typename T, typename... Args>
	void InitPool(T& f, std::string& constructName, Args... args)
	{
		int num = m_counter[constructName];

		if (num != 0)
		{
			for (int i = 0; i < num - 1; i++)
			{
				m_object_map.emplace(constructName, f(args...));
			}
			m_counter[constructName] = 0;
		}
	}

	template<typename T, typename... Args>
	std::shared_ptr<T> GetInstance(std::string& constructName, Args... args)
	{
		auto it = m_object_map.find(constructName);
		if (it == m_object_map.end())
			return nullptr;

		auto ptr = it->second.AnyCast<std::shared_ptr<T>>();
		if (sizeof...(Args) > 0)
			*ptr.get() = std::move(T(args...));

		m_object_map.erase(it);
		return ptr;
	}

private:
	std::multimap<std::string, Any> m_map;
	std::multimap<std::string, Any> m_object_map;
	std::map<std::string, int> m_counter;
	bool needClear;
};

struct AT
{
	AT() {}

	AT(int a, int b) :m_a(a), m_b(b) {}

	void Fun()
	{
		std::cout << m_a << " " << m_b << std::endl;
	}

	int m_a = 0;
	int m_b = 0;
};

struct BT
{
	void Fun()
	{
		std::cout << "from object b " << std::endl;
	}
};

void TestObjectPool()
{
	superObjectPool pool;
	pool.Create<AT>(2);
	pool.Create<BT>(2);
	pool.Create<AT, int, int>(2);

	{
		auto p = pool.Get<AT>();
		p->Fun(); //0 0
	}

	auto pb = pool.Get<BT>();
	pb->Fun();//from object b

	auto p = pool.Get<AT>();
	p->Fun();// 0 0

	int a = 5, b = 6;
	auto p2 = pool.Get<AT>(a, b);
	p2->Fun();// 5 6

	auto p3 = pool.Get<AT>(3, 4);
	p3->Fun();// 3 4

	{
		auto p4 = pool.Get<AT>(3, 4);
		p4->Fun();// 3 4 
	}
	auto p4 = pool.Get<AT>(7, 8);
	p4->Fun();// 7 8

}

//int main()
//{
//	TestObjectPool();
//}