#define USASERVER 0
#define EURSERVER 1

const int SERVERTYPE = USASERVER; // 0 = USA Server / 1 = EUR Server

struct f_date {
	unsigned year : 8;
	unsigned month : 5;
	unsigned day : 6;
	unsigned hour : 6;
	unsigned minute : 7;

	f_date(){year=0;month=0;day=0;hour=0;minute=0;};
};
