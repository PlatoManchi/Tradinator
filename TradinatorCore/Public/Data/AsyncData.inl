#include "Data/AsyncData.h"

#include <cassert>

template<typename T>
AsyncData<T>::AsyncData()
	: m_is_ready(false)
	, m_was_ready_before(false)
{

}


template<typename T>
T& AsyncData<T>::GetData()
{
	assert(m_is_ready && ("Data is not ready to access yet."));
	return m_data;
}

template<typename T>
T& AsyncData<T>::GetAsyncDataCopy()
{
	//assert(!m_is_ready && ("Cannot access async data if data is ready. Use GetData to get the data."));
	return m_async_data_copy;
}


template<typename T>
void AsyncData<T>::SetDataReady(bool is_ready)
{
	if (m_is_ready != is_ready)
	{
		if (is_ready)
		{
			m_log_to_file_mutex.lock();

			m_data = std::move(m_async_data_copy);
			m_was_ready_before = true;

			m_log_to_file_mutex.unlock();
		}
		else
		{
			// reset the copy to default value
			m_async_data_copy = T();
		}

		m_is_ready = is_ready;
	}
}

template<typename T>
bool AsyncData<T>::Reset()
{
	if (!m_is_ready)
	{
		return false;
	}

	m_is_ready = false;
	m_was_ready_before = false;

	m_data = T();
	m_async_data_copy = T();

	return true;
}