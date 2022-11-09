// objectpool.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

#include <iostream>
#include <memory>
#include <functional>
#include <vector>

template <class T>
class SimpleObjectPool
{
public:
	using DeleterType = std::function<void(T*)>;

	void add(std::unique_ptr<T> t)
	{
		pool_.push_back(std::move(t));
	}
	std::unique_ptr<T, DeleterType> get()
	{
		if (pool_.empty())
		{
			throw std::logic_error("no more object");
		}
		std::unique_ptr<T, DeleterType> ptr(pool_.back().release(), [this](T* t)
		{
			//DeleterType删除器，将对象回收到对象池中
			pool_.push_back(std::unique_ptr<T>(t));
		});

		//shared_ptr的做法，需要创建一个新对象，并且拷贝原来的对象
		//auto p = std::shared_ptr<T>(new T(*ptr.get()), [this](T* t)
		//{
		//	pool_.push_back(std::shared_ptr<T>(t));
		//});

		pool_.pop_back();
		return std::move(ptr);
	}

	bool empty() const
	{
		return pool_.empty();
	}

	size_t size() const
	{
		return pool_.size();
	}

private:
	std::vector<std::unique_ptr<T>> pool_;
};

void test_object_pool()
{
	SimpleObjectPool<int> p;
	p.add(std::unique_ptr<int>(new int()));
	p.add(std::unique_ptr<int>(new int()));
	{
		auto t = p.get();
		p.get();
	}
	{
		p.get();
		p.get();
	}
	std::cout << p.size() << std::endl; //2
}

//int main()
//{
//	test_object_pool();
//}

