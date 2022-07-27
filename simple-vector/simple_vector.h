#pragma once

#include <iostream>
#include <cassert>
#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include "array_ptr.h"

class ReserveProxyObj {
public:
    size_t new_cap = 0;
    ReserveProxyObj(size_t size)
        : new_cap(size)
    {
    }
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_(size),
          capacity_(size),
          items_(size)
    {
        for (size_t i = 0; i < size_; ++i) {
            items_[i] = Type();
        }
    }

    SimpleVector(ReserveProxyObj new_capacity) {
        Reserve(new_capacity.new_cap);
    }

    SimpleVector(size_t size, const Type& value)
        : size_(size),
          capacity_(size),
          items_(size)
    {
        std::fill(&items_[0], &items_[size_], value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()),
          capacity_(init.size()),
          items_(init.size())
    {
        std::copy(init.begin(), init.end(), &items_[0]);
    }

    SimpleVector(const SimpleVector& other) 
        : size_(other.GetSize()),
          capacity_(other.GetSize()),
          items_(other.GetSize())
    {
        ArrayPtr<Type> tmp(other.GetSize());
        std::copy(other.begin(), other.end(), &tmp[0]);
        items_.swap(tmp);
    }

    SimpleVector(SimpleVector&& other)
        : size_(other.GetSize()),
        capacity_(other.GetCapacity()),
        items_(other.GetSize()) {
        std::move(other.begin(), other.end(), begin());
        std::exchange(other.capacity_, 0);
        std::exchange(other.size_, 0);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0 ? true : false;
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return const_cast<Type&>(items_[index]);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        items_ = std::move(rhs.items_);
        size_ = rhs.size_;
        capacity_ = rhs.capacity_;
        rhs.capacity_ = 0;
        rhs.size_ = 0;
        return *this;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return const_cast<Type&>(items_[index]);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        return At(index);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        //Iterator result = items_.Get();
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        //Iterator result = &items_[size_];
        return &items_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return const_cast<Type*>(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return const_cast<Type*>(&items_[size_]);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return const_cast<Type*>(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return const_cast<Type*>(&items_[size_]);;
    }

    // Изменяет размер массива.
// При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size < capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                items_[i] = std::move(Type());
            }
            size_ = new_size;
            return;
        }
        if (new_size > capacity_) {
            SimpleVector<Type> new_items(new_size);
            new_items.size_ = new_size;
            std::move(begin(), end(), new_items.begin());
            for (size_t i = size_; i < new_size; ++i) {
                new_items.items_[i] = std::move(Type());
            }
            *this = std::move(new_items);
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = item;
    }

    void PushBack(Type&& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert((pos >= begin()) && (pos <= end()));
        if (capacity_ == 0) {
            PushBack(value);
            return Iterator(items_.Get());
        }
        auto distance_ = std::distance(cbegin(), pos);
        if (size_ + 1 > capacity_) {
            ArrayPtr<Type> tmp(capacity_ * 2);
            std::move(cbegin(), pos, &tmp[0]);
            std::move_backward(pos, cend(), &tmp[size_ + 1]);
            tmp[distance_] = value;
            items_.swap(tmp);
            capacity_ = capacity_ * 2;
            ++size_;
        }
        else {
            std::copy_backward(pos, cend(), &items_[size_ + 1]);
            items_[distance_] = value;
            ++size_;
        }
        return Iterator(&items_[distance_]);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert((pos >= begin()) && (pos <= end()));
        if (capacity_ == 0) {
            PushBack(std::move(value));
            return Iterator(&items_[0]);
        }
        auto new_element_position = std::distance(cbegin(), pos);
        Iterator new_pos = begin() + new_element_position;
        if (size_ + 1 > capacity_) {
            ArrayPtr<Type> tmp(capacity_ * 2);
            std::move(begin(), new_pos, &tmp[0]);
            tmp[new_element_position] = std::move(value);
            std::move_backward(new_pos, end(), &tmp[size_ + 1]);
            items_ = std::move(tmp);
            capacity_ = capacity_ * 2;
            ++size_;
        }
        else {
            std::move_backward(new_pos, end(), &items_[size_ + 1]);
            items_[new_element_position] = std::move(value);
            ++size_;
        }
        return Iterator(&items_[new_element_position]);
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(size_ > 0);
        assert((pos >= cbegin()) && (pos < cend()));
        auto distance_ = std::distance(cbegin(), pos);
        Iterator new_pos = begin() + distance_;
        std::move(new_pos + 1, end(), &items_[distance_]);
        --size_;
        return Iterator(&items_[distance_]);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            if (size_ > 0) {
                std::move(cbegin(), cend(), &tmp[0]);
            }
            items_.swap(tmp);
            capacity_ = new_capacity;
            return;
        }
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs, rhs);
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
