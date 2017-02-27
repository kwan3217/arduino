//DST.h
#ifndef DST_h
#define DST_h

//Hours the local time zone is behind UTC during standard time - positive for all US time zones
const int tz=7;
//Set to true if the location you are in uses DST - If so, code figures out if DST is in effect, if not, uses standard time year-round
const bool useDST=true;

//Input - time in local time zone
//  Year - 4 digit year
//  Month - month number, 1=January
//  Day - conventional calendar day of month number
//  hour - local hour in 24h format
//Return - true if in DST, false if not
bool isDST(int year, int month, int day, int hour);
//Local time variables
extern volatile signed long h,n,s,t,u,d,m,y;
//Copies time from GPS variables, in UTC, to local time variables above
void toLocalTime();

#endif

