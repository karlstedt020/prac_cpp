#include <iostream>

template <typename T>
class shared_ptr {
    T* ptr;
    int* counter = nullptr;

    template<typename T2, typename... Args>
    friend shared_ptr<T2>& make_shared(Args&&... args);

public:
    shared_ptr(T* ptr) {
        this->ptr = ptr;
        this->counter = new int{1};
    }

    shared_ptr(const shared_ptr<T>& other) {
        this->ptr = other.ptr;
        this->counter = other.counter;
        (*(this->counter))++;
        //std::cout << "Copy constructor" << std::endl;
    }

    shared_ptr(shared_ptr&& other) {
        this->ptr = other.ptr;
        this->counter = other.counter;
        other.ptr = nullptr;
        other.counter = nullptr;
    }

    T& operator*() {
        return *(this->ptr);
    }

    T* operator->() {
        return this->ptr;
    }

    T* get() {
        return this->ptr;
    }

    operator bool() {
        return this->counter != nullptr;
    }

    T& operator[](int idx) {
        return (this->ptr)[idx];
    }

    int use_count() {
        return *(this->counter);
    }

    shared_ptr<T>& operator=(const shared_ptr<T>& other) {
        if (this->ptr == other.ptr)
            return *this;
        if (*(this->counter) == 1) {
            delete this->counter;
            delete this->ptr;
        }
        (*(this->counter))--;
        this->ptr = other.ptr;
        this->counter = other.counter;
        (*(this->counter))++;
        return *this;
    }

    ~shared_ptr() {
        //std::cout << "destr" << std::endl;
        if ((*counter) > 1) {
            (*counter)--;
        }
        else {
            delete counter;
            delete ptr;
        }
    }
    // reset, swap, comparison
    void reset() {
        if (*counter == 1) {
            delete counter;
            delete ptr;
            return;
        }
        (*counter)--;
        counter = nullptr;
        ptr = nullptr;
    }

    void reset(const T* ptr) {
        if (*counter == 1) {
            delete counter;
            delete ptr;
            return;
        }
        (*counter)--;
        counter = new int{1};
        this->ptr = ptr;
    }

    void swap(shared_ptr<T>& other) {
        swap(this->counter, other.counter);
        swap(this->ptr, other.ptr);
    }

    bool operator<(shared_ptr<T> &other) {
        return *(this->ptr) < *(other.ptr);
    }

    bool operator==(shared_ptr<T> &other) {
        return *(this->ptr) == *(other.ptr);
    }

    bool operator!=(shared_ptr<T> &other) {
        return *(this->ptr) != *(other.ptr);
    }

    bool operator>(shared_ptr<T> &other) {
        return *(this->ptr) > *(other.ptr);
    }
};

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
    T* obj = new T{std::forward<Args...>(args...)};
    return shared_ptr<T>(obj);
}

//int main() {
//    auto a = make_shared<int>(1), b = make_shared<int>(2);
//    std::cout << (a > b);
//}
