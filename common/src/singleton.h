#ifndef SINGLETON_H
#define SINGLETON_H

/**
 * 线程安全的单例模板类
 * @tparam T 单例类类型
 */
template <typename T>
class Singleton {
public:
    static T& instance() {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;
};

#define DECLARE_SINGLETON(ClassName)                \
private:                                            \
    friend class Singleton<ClassName>;              \
public:                                             \
    static ClassName& instance() {                  \
        return Singleton<ClassName>::instance();    \
    }

#endif //SINGLETON_H
