
class Singleton{
public:


private:
	static Singleton* instance;
	
	Singleton(){}
	
	class Deleter{
	public:
		~Deleter()
		{
			if(Singleton::instance)
				delete Singleton::instance;
			Singleton::instance=NULL;
		}
	};
	static Deleter del;
};

Singleton::Deleter Singleton::del;