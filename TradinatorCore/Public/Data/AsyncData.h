#pragma once

#include <thread>
#include <mutex>

template <typename T>
class AsyncData
{
public:
	AsyncData();
	AsyncData(std::thread::id tradinator_core_thread_id);

	AsyncData(const AsyncData& other) = default;
	AsyncData(AsyncData&& other) noexcept = default;
	AsyncData& operator=(const AsyncData& other) = default;
	AsyncData& operator=(AsyncData&& other) noexcept = default;

	T& GetData();
	T& GetAsyncDataCopy();

	// Const versions to access data from other thread as cached version of the data.
	// Careful when calling these functions
	const T& GetData() const { return m_data; };
	const T& GetAsyncDataCopy() const { return m_async_data_copy; };

	void SetDataReady(bool is_ready);
	inline bool IsDataReady() const { return m_is_ready; };
	inline bool WasEverReadyBefore() const { return m_was_ready_before; }

	

private:
	// Has actual data. When async process is loading the data this is untouched
	T m_data;

	// Data that async tasks and threads work on. When data is set to ready for access
	// data from this variable is moved into m_data for access;
	T m_async_data_copy;

	// If data is ready to be accessed.
	bool m_is_ready = true;

	// Store if this data was ever processed successfully before.
	// If it was, then m_data can be considered as cached data from previous processing.
	bool m_was_ready_before = false;

	std::mutex m_mutex;

	// The ID of thread TradinatorCore is constructed on.
	// Caching and using this to make debugging easy
	const std::thread::id m_tradinator_core_thread_id;
};




#include "Data/AsyncData.inl"