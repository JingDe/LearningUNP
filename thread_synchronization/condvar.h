class CondVar {
public:
	explicit CondVar(Mutex* mu);
	~CondVar();

	void Wait();
	void waitForSeconds(int secs);
	void Signal();
	void SignalAll();

private:
	Mutex * mu_;

	pthread_cond_t cv_;
};


CondVar::CondVar(Mutex* mu) :mu_(mu)
{
	PthreadCall("Init cv", pthread_cond_init(&cv_, NULL));
	//cv_=PTHREAD_COND_INITIALIZER;
}

CondVar::~CondVar()
{
	PthreadCall("Destroy cv", pthread_cond_destroy(&cv_));
}

void CondVar::Wait() 
{
	// 释放锁
	PthreadCall("wait", pthread_cond_wait(&cv_, &mu_->mu_));
}

void CondVar::waitForSeconds(int secs)
{
	struct timespec abstime;
	/*struct timespec{
		time_t tv_sec;
		long tv_nsec;
	};*/
	clock_gettime(CLOCK_REALTIME, &abstime); // 获得特定clock的时间

	const int64_t kNanoSecondsPerSecond = 1e9;
	int64_t nanoseconds = static_cast<int64_t>(secs *kNanoSecondsPerSecond);

	abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
	abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

	PthreadCall("wait for seconds", pthread_cond_timedwait(&cv_, &(mu_->mu_), &abstime)); // 等待到一个绝对时间
}

void CondVar::Signal()
{
	PthreadCall("signal", pthread_cond_signal(&cv_));
}

void CondVar::SignalAll()
{
	PthreadCall("broadcast", pthread_cond_broadcast(&cv_));
}