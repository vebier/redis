
#ifndef REDIS_SINGLETON_H
#define REDIS_SINGLETON_H
#include <memory>
#include <mutex>

template<typename T>
class Singleton {
public:
    static std::shared_ptr<T> instance(){

        static std::once_flag flag;
        std::call_once(flag, [](){
            instance_ = std::shared_ptr<T>(new T());
        });
        return instance_;
    }
private:
    static std::shared_ptr<T> instance_;
};
template<typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;

#endif //REDIS_SINGLETON_H
