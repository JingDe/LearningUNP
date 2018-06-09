

class Singleton{
public:
	static Singleton* getInstance()
	{
		{
			MutexLock lock(mu_);
			static Singleton instance;
		}
		return &instance;
	}
	
private:
	Singleton(){}
	
	Mutex mu_;
};

class Singleton{
public:
	static Singleton getInstance()
	{
		{
			MutexLock lock(mu_);
			static Singleton instance;
		}
		return instance;
	}
	
private:
	Singleton(){}
	
	Mutex mu_;
	
	// 禁止拷贝:Singleton singleton = Singleton :: GetInstance();
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);
};






