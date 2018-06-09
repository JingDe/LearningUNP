class MutexLock {
public:
	MutexLock(Mutex& mu) :mu_(mu) {
		mu_.Lock();
	}
	~MutexLock() {
		mu_.Unlock();
	}

private:
	Mutex & mu_; 
};