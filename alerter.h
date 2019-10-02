#ifndef _wt_alerter_h_
#define _wt_alerter_h_

#include <functional>

void StartAlert(std::function<void(void)> cb);
void StopAlert();
bool IsAlertActive();


#endif
