
// 双重检查加锁
class Singleton{
public:
	static Singleton* getInstance()
	{
		Singleton* tmp=NULL;
		
		asm ("memoryBarrier");
		if(instance==NULL)
		{
			{
				MutexLock lock(mu_); // 只在第一次加锁来初始化
				if(instance==NULL) 
				{
					//instance=new Singleton();
					tmp=new Singleton();
					asm ("memoryBarrier"); // 保证在内部初始化之后对instance赋值
					instance=tmp;
				}
			}
		}
		return instance;
	}
	
private:
	static Singleton* instance;
	
	Singleton(){}
	
	Mutex mu_;
};

Singleton *Singleton::instance=NULL;