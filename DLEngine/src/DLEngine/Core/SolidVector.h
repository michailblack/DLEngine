#pragma once
#include "DLEngine/Core/Base.h"

#include <vector>

template <typename T>
class SolidVector
{
public:
    using ID = uint32_t;
    using Index = uint32_t;

    bool occupied(ID id) const noexcept { DL_ASSERT_NOINFO(id < static_cast<ID>(m_Occupied.size())); return m_Occupied[id]; }

    Index size() const noexcept { return static_cast<Index>(m_Data.size()); }

    const T* data() const noexcept { return m_Data.data(); }
    T* data() noexcept { return m_Data.data(); }

    const T& at(Index index) const noexcept { DL_ASSERT_NOINFO(index < size()); return m_Data[index]; }
    T& at(Index index) noexcept { DL_ASSERT_NOINFO(index < size()); return m_Data[index]; }

    const T& operator[](ID id) const noexcept { DL_ASSERT_NOINFO(occupied(id)); return m_Data[m_ForwardMap[id]]; }
    T& operator[](ID id) noexcept { DL_ASSERT_NOINFO(occupied(id)); return m_Data[m_ForwardMap[id]]; }

    Index getIndex(ID id) const noexcept { DL_ASSERT_NOINFO(occupied(id)); return m_ForwardMap[id]; }

    ID insert(const T& value) noexcept
    {
        ID id{ m_NextUsed };

        DL_ASSERT_NOINFO(id <= static_cast<ID>(m_ForwardMap.size()) && m_ForwardMap.size() == m_Occupied.size());

        if (id == m_ForwardMap.size())
        {
            m_ForwardMap.push_back(static_cast<Index>(m_ForwardMap.size()) + 1u);
            m_Occupied.push_back(false);
        }

        DL_ASSERT_NOINFO(!m_Occupied[id]);

        m_NextUsed = m_ForwardMap[id];
        m_ForwardMap[id] = static_cast<Index>(m_Data.size());
        m_Occupied[id] = true;

        m_Data.emplace_back(value);
        m_BackwardMap.emplace_back(id);

        return id;
    }

    void erase(ID id) noexcept
    {
        DL_ASSERT_NOINFO(id <= static_cast<ID>(m_ForwardMap.size()) && m_ForwardMap.size() == m_Occupied.size());
        DL_ASSERT_NOINFO(occupied(id));

        Index& forwardIndex{ m_ForwardMap[id] };

        m_Data[forwardIndex] = std::move(m_Data.back());
        m_Data.pop_back();

        ID backwardIndex{ m_BackwardMap.back() };

        m_BackwardMap[forwardIndex] = backwardIndex;
        m_BackwardMap.pop_back();

        m_ForwardMap[backwardIndex] = forwardIndex;

        forwardIndex = m_NextUsed;
        m_Occupied[id] = false;
        m_NextUsed = id;
    }

    void clear() noexcept
    {
        m_Data.clear();
        m_ForwardMap.clear();
        m_BackwardMap.clear();
        m_Occupied.clear();
        m_NextUsed = 0u;
    }

    void reserve(Index capacity) noexcept
    {
        m_Data.reserve(capacity);
        m_ForwardMap.reserve(capacity);
        m_BackwardMap.reserve(capacity);
        m_Occupied.reserve(capacity);
    }

    typename std::vector<T>::iterator begin() noexcept { return m_Data.begin(); }
    typename std::vector<T>::iterator end() noexcept { return m_Data.end(); }

    typename std::vector<T>::const_iterator begin() const noexcept { return m_Data.begin(); }
    typename std::vector<T>::const_iterator end() const noexcept { return m_Data.end(); }

public:
    std::vector<T> m_Data;
    std::vector<Index> m_ForwardMap;
    std::vector<ID> m_BackwardMap;
    std::vector<bool> m_Occupied;

    ID m_NextUsed{ 0u };
};
