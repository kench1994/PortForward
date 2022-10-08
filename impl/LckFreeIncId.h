#pragma once
#include <atomic>
#include <limits>

namespace utils
{

	template <typename T,
		typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
		class LckFreeIncId
	{
	public:
		LckFreeIncId(T maxVal = 0)
			: m_atID(1)
			, SecondMaxVal(maxVal ? maxVal - 1 : (std::numeric_limits<T>::max)() - 1)
		{}
		virtual ~LckFreeIncId() {}



		T increase()
		{
			auto id = m_atID.fetch_add(1, std::memory_order::memory_order_acquire);
			if (SecondMaxVal == id)
			{
				m_atID.store(1, std::memory_order_release);
				return 1;
			}
			return id;
		}

	private:
		std::atomic<T> m_atID;
		T SecondMaxVal;
	};
}
