#pragma once
#include "ConstantMath.hpp"
#include <vector>
#include <algorithm>
#include <cassert>

template<size_t elementSize>
struct alignas(elementSize) Index {};

template<size_t elementSize, class IndexType, 
    size_t chunkSize = Pow(2, Ceil(Log2(elementSize)))>
class DynamicAllocator {

private:

    using chunkType = Index<chunkSize>;
    chunkType* chunk;

    IndexType freeListHead = 0;
    IndexType terminationValue = -1;
    size_t capacity;

    template<class T, class... Args>
    T* add_at(IndexType index, Args&&... args) {
        return static_cast<T*>(new (chunk + index) T(std::forward<Args>(args)...));
    }

#pragma region Checks
    template<class T>
    bool within_size_limit() const { return sizeof(T) <= elementSize; }
    template<class T>
    bool is_in_memory(T* object) const { return (int)capacity >= (chunkType*)object - chunk; }
#pragma endregion



public:

#pragma region Constructors

    explicit DynamicAllocator(size_t capacity) : capacity(capacity), chunk(new chunkType[capacity]){      
        for (IndexType i = 0; i < capacity; i++)
            add_at<IndexType>(i, i == capacity - 1 ? terminationValue : i + 1);
    }

    ~DynamicAllocator() noexcept {
        delete[] chunk;
    }

    DynamicAllocator(DynamicAllocator&) = delete;
    DynamicAllocator& operator=(DynamicAllocator&) = delete;
#pragma endregion

    template<class T, class... Args>
    T* create(Args... args) {
        assert(within_size_limit<T>());
        try
        {
            IndexType nextHead = at<IndexType>(freeListHead);
            auto pointer = add_at<T>(freeListHead, std::forward<Args>(args)...);
            freeListHead = nextHead;
            return pointer;
        }
        catch (const std::bad_alloc&){
            std::cout << "Pool is out of memory: bad_alloc " << std::endl;
            throw;
        }
    }


    template<class T>
    void destroy(T* object) {
        assert(is_in_memory<T>(object));
        IndexType index = (chunkType*)object - chunk;
        object->~T();
        add_at<IndexType>(index, freeListHead);
        freeListHead = index;
    }

    template<class Return>
    Return at(const IndexType index) const {
        return *(Return*)(chunk + index);
    }

#pragma region Visitor Functions
    template<class F>
    void freelist_visitor(F&& f) const {
        IndexType nextFree = freeListHead;
        while (nextFree != terminationValue)
        {
            f(nextFree);
            nextFree = at<IndexType>(nextFree);
        }
    }
    
    int count_free() const {
        int freeCounter = 0;
        freelist_visitor([&freeCounter]([[maybe_unused]] IndexType i){++freeCounter; });
        return freeCounter;
    }

    void dump_pool() const {
        using std::cout;
        auto freeSlots = std::vector<IndexType>();
        freelist_visitor([&freeSlots](IndexType i) {freeSlots.push_back(i); });
        std::sort(freeSlots.rbegin(), freeSlots.rend());

        cout << "Pool content: ";
        for (IndexType i = 0; i < capacity; i++)
        {
            cout << '[';
            if (!freeSlots.empty() && *freeSlots.rbegin() == i) {
                cout << at<IndexType>(i);
                freeSlots.pop_back();
            }
            else {
                cout << "X";
            }
            cout << ']';
        }
        cout << " Capacity: " << capacity << " Free slots: " << count_free();
        cout << " Current freelist head: " << freeListHead << std::endl;
    }
#pragma endregion
};