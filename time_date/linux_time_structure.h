
<1>
struct timeval{
	time_t tv_sec;
	suseconds_t tv_usec; //微秒
};

int gettimeofday(struct timeval *tv, struct timezone *tz);

例：
struct timeval now_tv;
gettimeofday(&now_tv, NULL);

<2>
time_t
time_t time(time_t* t);
time_t mktime (struct tm * timeptr);

例：
const time_t seconds=now_tv.tv_sec;


<3>
struct tm
{
    int tm_sec;  /*秒，正常范围0-59， 但允许至61*/
    int tm_min;  /*分钟，0-59*/
    int tm_hour; /*小时， 0-23*/
    int tm_mday; /*日，即一个月中的第几天，1-31*/
    int tm_mon;  /*月， 从一月算起，0-11*/  1+p->tm_mon;
    int tm_year;  /*年， 从1900至今已经多少年*/  1900＋ p->tm_year;
    int tm_wday; /*星期，一周中的第几天， 从星期日算起，0-6*/
    int tm_yday; /*从今年1月1日到目前的天数，范围0-365*/
    int tm_isdst; /*日光节约时间的旗标*/
};

struct tm* gmtime(const time_t* timep);
struct tm *localtime(const time_t *clock);
struct tm* localtime_r( const time_t* timer, struct tm* result );
size_t strftime (char* ptr, size_t maxsize, const char* format,const struct tm* timeptr );

例：
struct tm t;
localtime_r(&seconds, &t);
p+=snprintf(p, limit-p, "%04d/%02d/%02d-%02d:%02d:%02d.%06d %llx",
			t.tm_year+1900,
			t.tm_mon+1,
			t.tm_mday,
			t.tm_hour,
			t.tm_min,
			t.tm_sec,
			static_cast<int>(now_tv.tv_usec),
static_cast<long long unsigned int>(thread_id));


<4>
struct timespec {
time_t tv_sec; // seconds 
long tv_nsec; // and nanoseconds 
};

int clock_gettime(clockid_t, struct timespec *)
