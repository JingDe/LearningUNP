

// wait端
Mutex mu;
CondVar cond(mu);
std::queue<int> que;

int dequeue()
{
	MutexLock lock(&mu);
	while(que.empty())
	{
		cond.wait();
	}
	
	int top=que.top();
	que.pop();
	return top;
}

// signal、broadcast端
void produce(int ele)
{
	MutexLock lock(&mu);
	que.push(ele);
	cond.signal();
}