#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <typeindex>
struct Any
{
	Any(void) : m_tpIndex(std::type_index(typeid(void))) {} 
	Any(const Any& that) : m_ptr(that.Clone()), m_tpIndex(that.m_tpIndex) {} //��������
	Any(Any && that) : m_ptr(std::move(that.m_ptr)), m_tpIndex(that.m_tpIndex) {} //�ƶ�����

	//��������ָ��ʱ������һ������ͣ�ͨ��std::decay���Ƴ����ú�cv�����Ӷ���ȡԭʼ����
	template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Any>::value, U>::type> 
		Any(U && value) : m_ptr(new Derived < typename std::decay<U>::type>(std::forward<U>(value))),
		m_tpIndex(std::type_index(typeid(typename std::decay<U>::type))) {}

	bool IsNull() const { return !bool(m_ptr); }

	template<class U> bool Is() const
	{
		return m_tpIndex == std::type_index(typeid(U));
	}

	//��Anyת��Ϊʵ�ʵ�����
	template<class U>
	U& AnyCast()
	{
		if (!Is<U>())
		{
			std::cout << "can not cast " << typeid(U).name() << " to " << m_tpIndex.name() << std::endl;
			throw std::bad_cast();
		}

		auto derived = dynamic_cast<Derived<U>*> (m_ptr.get());
		return derived->m_value;
	}

	Any& operator=(const Any& a)
	{
		if (m_ptr == a.m_ptr)
			return *this;

		m_ptr = a.Clone();
		m_tpIndex = a.m_tpIndex;
		return *this;
	}

private:
	struct Base;
	typedef std::unique_ptr<Base> BasePtr;

	struct Base
	{
		virtual ~Base() {}
		virtual BasePtr Clone() const = 0;
	};

	template<typename T>
	struct Derived : Base
	{
		template<typename U>
		Derived(U && value) : m_value(std::forward<U>(value)) { }

		//��¡�����ഴ�������ظ�������ָ��
		BasePtr Clone() const
		{
			return BasePtr(new Derived<T>(m_value));
		}

		T m_value;
	};

	BasePtr Clone() const
	{
		if (m_ptr != nullptr)
			return m_ptr->Clone();

		return nullptr;
	}

	BasePtr m_ptr;
	std::type_index m_tpIndex; //��ʶ����
};
//���˼·��Any�ڲ�ά����һ������ָ�룬ͨ������ָ������������ͣ�any_castʱ��ͨ������ת�ͻ�ȡʵ�����ݡ���ת��ʧ��ʱ��ӡ���顣
void TestAny()
{
	Any n;
	auto r = n.IsNull();//true
	std::string s1 = "hello";
	n = s1;
	n = "world";
	//n.AnyCast<int>(); //can not cast int to string
	Any n1 = 1;
	std::cout << n1.Is<int>() << std::endl;//true
}

