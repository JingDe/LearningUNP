

namespace{
template<typename T>
struct has_no_destroy
{
	// 当C有no_destroy成员，调用此版本
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template <typename C> static char test(decltype(&C::no_destroy));
#else
  template <typename C> static char test(typeof(&C::no_destroy));
#endif
	
	template<typename C> static int32_t test(...);// 当C没有no_destroy成员，调用此版本
	
	
	const static bool value=sizeof(test<T>(0))==1;// 当T没有no_destroy成员，value=false
};
}

template<typename T>
class Singleton
{
public:
	static T& instance()
	{
		pthread_once(&ponce_, &Singleton::init);
		//assert(value_ != NULL);
		return *value_;
	}
	
private:
	Singleton();
	~Singleton();
	
	static void init()
	{
		value_=new T();
		if(!detail::has_no_destroy<T>::value)
		{
			atexit(destroy);
		}
	}
	
	static void destroy()
	{
		typedef char T_must_be_complete_type[sizeof(T)==0  ?  -1  :  1];
		T_must_be_complete_type dummy;
		
		delete value_;
		value_=NULL;
	}
	
	static pthread_once_t ponce_;
	static T* value_;
	
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);
};

template<typename T>
pthread_once_t Singleton<T>::ponce_=PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::value_=NULL;

//用户
//Foo& foo=Singleton<Foo>::instance();
