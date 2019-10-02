
#include "alerter.h"
#include <ctime>
#include <thread>

#include "utilfunc.h"

bool bAlertStop;
bool bAlertUp=false;

void alert_thread(std::function<void(void)> cb)
{
	struct timespec ts;
	int c=0;
	std::function<void(void)> CB=cb;
	bAlertUp=true;
	while (!bAlertStop)
	{
		ts.tv_sec=1; //0; /* seconds */
		ts.tv_nsec=0; //500000000; /* nanoseconds (1/1000000 second)*/
		nanosleep(&ts, NULL);
		c++; if (c>60) { if (CB) CB(); c=0; } //once per minute
	}
	bAlertUp=false;
}

void StartAlert(std::function<void(void)> cb)
{
	if (IsAlertActive()) StopAlert();
	bAlertUp=bAlertStop=false;
	std::thread(alert_thread, cb).detach();
}

void StopAlert()
{
	int i=0;
	bAlertStop=true;
	while (bAlertUp&&(i<100)) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); i++; }
	//Message(uspf("alerter: i = ", i)); //alert-thread sleeps for 1 sec each loop
}

bool IsAlertActive() { return bAlertUp; }

