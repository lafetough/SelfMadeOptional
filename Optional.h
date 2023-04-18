#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    Optional(const T& value) {
        new(data_) T(value);
        is_initialized_ = true;
    }
    Optional(T&& value) {
        new(data_) T(std::move(value));
        is_initialized_ = true;
    }
    Optional(const Optional& other) {
        if (other.is_initialized_) {
            new(data_) T(*reinterpret_cast<const T*>(other.data_));
            is_initialized_ = true;
        }
    }
    Optional(Optional&& other) {
        if (other.is_initialized_) {
            new(data_) T(std::move(*reinterpret_cast<T*>(other.data_)));
            is_initialized_ = true;
        }
    }

    Optional& operator=(const T& value) {
        if (!is_initialized_) {
            new(data_) T(value);
            is_initialized_ = true;
        }
        else {
            *reinterpret_cast<T*>(data_) = value;
        }
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (!is_initialized_) {
            new(data_) T(std::move(rhs));
            is_initialized_ = true;
        }
        else {
            *reinterpret_cast<T*>(data_) = std::move(rhs);
        }
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (is_initialized_ && rhs.is_initialized_) {
            *reinterpret_cast<T*>(data_) = *reinterpret_cast<const T*>(rhs.data_);
        }
        else if (!is_initialized_ && rhs.is_initialized_) {
            new(data_) T(*reinterpret_cast<const T*>(rhs.data_));
            is_initialized_ = true;
        }
        else if (is_initialized_ && !rhs.is_initialized_) {
            reinterpret_cast<T*>(data_)->~T();
            is_initialized_ = false;
        }
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (is_initialized_ && rhs.is_initialized_) {
            *reinterpret_cast<T*>(data_) = std::move(*reinterpret_cast<T*>(rhs.data_));
        }
        else if (!is_initialized_ && rhs.is_initialized_) {
            new(data_) T(std::move(*reinterpret_cast<T*>(rhs.data_)));
            is_initialized_ = true;
        }
        else if (is_initialized_ && !rhs.is_initialized_) {
            reinterpret_cast<T*>(data_)->~T();
            is_initialized_ = false;
        }
        return *this;
    }

    ~Optional() {
        if (is_initialized_) {
            reinterpret_cast<T*>(data_)->~T();
        }
    }

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() {
        return *reinterpret_cast<T*>(data_);
    }
    const T& operator*() const {
        return *reinterpret_cast<const T*>(data_);
    }
    T* operator->() {
        return reinterpret_cast<T*>(data_);
    }
    const T* operator->() const {
        return reinterpret_cast<const T*>(data_);
    }

    //// Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value() {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return *reinterpret_cast<T*>(data_);
    }
    const T& Value() const {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return *reinterpret_cast<const T*>(data_);
    }

    void Reset() {
        if (is_initialized_) {
            reinterpret_cast<T*>(data_)->~T();
            is_initialized_ = false;
        }
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};