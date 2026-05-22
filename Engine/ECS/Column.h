#pragma once

#include <vector>
#include <memory>

// Column is used to define array of components
struct IColumn
{
	virtual ~IColumn() = default;
	virtual void RemoveSwapLast(uint32_t index) = 0;

	virtual void CopyFrom(IColumn* source, uint32_t srcIndex, uint32_t dstIndex) = 0;
};

template<typename T>
struct Column : public IColumn
{
	std::vector<T> data;

	void RemoveSwapLast(uint32_t index) override
	{
		data[index] = std::move(data.back());
		data.pop_back();
	}

	void Push(const T& value)
	{
		data.push_back(value);
	}

	T& Get(uint32_t index)
	{
		return data[index];
	}

	void CopyFrom(IColumn* source, uint32_t sourceIndex, uint32_t destinationIndex) override
	{
		Column<T>* sourceColumn = static_cast<Column<T>*>(source);
		if (data.size() <= destinationIndex)
		{
			data.resize(destinationIndex + 1);
		}

		data[destinationIndex] = sourceColumn->data[sourceIndex];
	}
};