
// 次次加锁
class Singleton{
public:
	static Singleton* getInstance()
	{
		if(instance==NULL)
			instance=new Singleton();
		return instance;
	}
	
private:
	static Singleton* instance;
	
	Singleton(){}
	// 禁止拷贝
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);
};

Singleton *Singleton::instance=NULL;

thread1(){
	
	{
		MutexLock l(mu_);
		Singleton* sing=Singleton::getInstance();
	}
}

thread2(){
	
	{
		MutexLock l(mu_);
		Singleton* sing=Singleton::getInstance();
	}
}
