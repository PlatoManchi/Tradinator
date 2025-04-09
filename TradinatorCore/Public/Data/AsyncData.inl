#include "Data/AsyncData.h"

#include <cassert>

template<typename T>
AsyncData<T>::AsyncData()
	: m_is_ready(true)
	, m_was_ready_before(false)
	, m_tradinator_core_thread_id()
{

}

template<typename T>
AsyncData<T>::AsyncData(std::thread::id tradinator_core_thread_id)
	: m_is_ready(true)
	, m_was_ready_before(false)
	, m_tradinator_core_thread_id(tradinator_core_thread_id)
{

}


template<typename T>
T& AsyncData<T>::GetData()
{
	assert(m_is_ready && ("Data is not ready to access yet."));
	//assert((m_tradinator_core_thread_id == std::this_thread::get_id()) && "Cannot access data outside main thread.");
	return m_data;
}

template<typename T>
T& AsyncData<T>::GetAsyncDataCopy()
{
	//assert(!m_is_ready && ("Cannot access async data if data is ready. Use GetData to get the data."));
	//assert((m_tradinator_core_thread_id != std::this_thread::get_id()) && "Cannot access async data copy in main thread.");
	return m_async_data_copy;
}


template<typename T>
void AsyncData<T>::SetDataReady(bool is_ready)
{
	if (m_is_ready != is_ready)
	{
		if (is_ready)
		{
			m_mutex.lock();

			m_data = std::move(m_async_data_copy);
			m_was_ready_before = true;

			m_mutex.unlock();
		}
		else
		{
			// reset the copy to default value
			m_async_data_copy = T();
		}

		m_is_ready = is_ready;
	}
}