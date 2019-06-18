#pragma once

#include <ntdef.h>

#include "new.hpp"

class String
{
	size_t size_ = 0;
	char* buffer_ = nullptr;

public:
	String() = default;

	explicit String(const char* string)
	{
		allocate(string);
	}

	String(const char* string, size_t size)
	{
		allocate(string, size);
	}

	void allocate (const char* string)
	{
		

		//                               null byte
		const size_t new_size = strlen(string) + 1;
		char* new_buffer = new char[new_size];
		strcpy_s(new_buffer, new_size, string);

		if (buffer_ != nullptr) delete[] buffer_;

		size_ = new_size;
		buffer_ = new_buffer;
	}

	void allocate(const char* string, size_t size)
	{
		if (buffer_ != nullptr) delete[] buffer_;

		size_ = size;
		buffer_ = new char[size_];
		strcpy_s(buffer_, size_, string);
	}

	void allocate(PCUNICODE_STRING string)
	{
		if (buffer_ != nullptr) delete[] buffer_;

		size_ = RtlxUnicodeStringToAnsiSize(string);
		buffer_ = new char[size_];

		ANSI_STRING str;
		str.MaximumLength = static_cast<USHORT>(size_);
		str.Buffer = buffer_;

		if (RtlUnicodeStringToAnsiString(&str, string, FALSE) == STATUS_SUCCESS) return;

		size_ = 0;
		delete[] buffer_;
		buffer_ = nullptr;
	}

	const char* buffer() const
	{
		return buffer_;
	}

	~String()
	{
		if (buffer_ == nullptr) return;
		delete[] buffer_;
	}
};