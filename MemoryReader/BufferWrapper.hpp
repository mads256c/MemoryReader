#pragma once
#include "Util.h"

class BufferWrapper
{

	char* buffer_ = nullptr;
	size_t size_;

public:

	explicit BufferWrapper(const size_t size) : size_(size)
	{
		buffer_ = new char[size];
	}

	template <typename T>
	[[nodiscard]] T* buffer() const
	{
		return (T*)buffer_;
	}

	[[nodiscard]] size_t size() const
	{
		return size_;
	}

	~BufferWrapper()
	{
		if (buffer_ == nullptr)
		{
			DebugLog("BufferWrapper: Buffer is null");
			return;
		}

		delete buffer_;
		buffer_ = nullptr;
	}
};