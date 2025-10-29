//用来让别的类继承，确保继承该模板的类在一个类在程序中只有唯一实例，并提供全局访问点

#ifndef SINGLETON_H
#define SINGLETON_H
#include <memory>
#include <mutex>
#include <iostream>
using namespace std;
template <typename T>//定义泛型编程的模板
class singleton
{
protected:
    singleton()=default;
    singleton(const singleton<T>&)=delete;//禁止拷贝构造函数
    singleton operator=(const singleton<T>& st)=delete;//禁止使用赋值运算符
    static std::shared_ptr<T> _instance;//声明一个静态的共享智能指针成员变量
public:
    //线程安全，节省资源，自动内存管理
    static std::shared_ptr<T> GetInstance(){//获取单例实例的接口，返回一个T类型对象的共享2只能指针
        static std::once_flag s_flag;//确保该代码只执行一次
        std::call_once(s_flag,[&](){//确保线程安全，保证其第二个参数（一段代码）无论多少线程同时调用，都只会被执行一次。
            _instance=shared_ptr<T>(new T);
        });
        return _instance;
    }
    void PrintAddress(){
        std::cout<<_instance.get()<<endl;
    }
    ~singleton(){//折构函数，在对象被销毁时调用
        std::cout<<"this is singleton destruct"<<endl;
    }
};
template <typename T>
std::shared_ptr<T> singleton<T>::_instance=nullptr;
#endif // SINGLETON_H
