#include <Arduino.h>
#include "DST.h"
#include "RMC.h"

volatile signed long h=0,n=0,s=0,d=0,m=0,y=0;

//Given the current year/month/day
//Returns 0 (Sunday) through 6 (Saturday) for the day of the week
//From: http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
//This function assumes the month from the caller is 1-12
int day_of_week(int year, int month, int day) {
  //Devised by Tomohiko Sakamoto in 1993, it is accurate for any Gregorian date:
  static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4  };
  year -= month < 3;
  //This works fine for day=0 or negative (for days before 1st of month), also days after end of month (feb 31st etc)
  return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7; 
}

//Implements the United States Daylight Saving Time rule. This code will remain valid until
//Congress in its wisdom decides to change the rules (again). The last time they changed
//was in 2007. At that time, the rule became:
//DST goes into effect at 2:00am local time on the second Sunday in March
//Standard time goes back into effect at 2:00am local time on the first Sunday in November
//During the change in March, this has the effect of the time skipping from 01:59:59am
//to 03:00:00am, meaning that the 2:00 hour is skipped. During the change in November,
//this has the effect of the time skipping from 01:59:59am to 01:00:00am, meaning 
//that the 1:00 hour *is repeated!*
//Input - time in local time zone
//  Year - 4 digit year
//  Month - month number, 1=January
//  Day - conventional calendar day of month number
//  hour - local hour in 24h format
//Return - true if in DST, false if not
bool isDST(int year, int month, int day, int hour) {
  bool result=false;
  const bool verbose=false;
  if(verbose){
    Serial.print(year);Serial.print(',');
    Serial.print(month);Serial.print(',');
    Serial.print(day);Serial.print(',');
    Serial.println(hour);
  }
  if(useDST) {
    //Is DST in effect?
    if(month < 3) { //Before March, not DST
      result=false;
      if(verbose)Serial.println("Before March, not DST");
    } else if (month>11) { //After November, not DST
      result=false;
      if(verbose)Serial.println("After November, not DST");
    } else if (month>3 && month<11) { //After march and before November, in DST
      result=true;
      if(verbose)Serial.println("After March and before November, in DST");
    } else if(month==3) { //In March, figure out what day is the second Sunday in March
      int firstDayOfMarch=day_of_week(year,3,1);
      if(firstDayOfMarch==0)firstDayOfMarch=7;
      //What day is March 1?   Number    Second sunday
      //         Sunday           0 (7)              8  (15-n)
      //         Monday           1                 14  (15-n)
      //         Tuesday          2                 13  (15-n)
      //         Wednesday        3                 12  (15-n)
      //         Thurdsay         4                 11  (15-n)
      //         Friday           5                 10  (15-n)
      //         Saturday         6                  9  (15-n)
      int secondSunday=15-firstDayOfMarch;
      if(verbose) {
        Serial.print("In March ");
        Serial.print(" day=");Serial.print(day);
        Serial.print(" secondSunday=");Serial.print(secondSunday);
      }
      if(day<secondSunday) {
        result=false;
        if(verbose)Serial.println("Before second Sunday, not DST");
      } else if(day>secondSunday) {
        result=true;
        if(verbose)Serial.println("After second Sunday, in DST");
      } else {
        result=(hour>=2);
        if(verbose){Serial.print("On second Sunday, hour=");Serial.print(hour);Serial.print(", DST is");Serial.println(result?"on":"off");}
      }
    } else { //Only November is left;            
      int firstDay=day_of_week(year,3,1);
      if(firstDay==0)firstDay=7;
      //What day is November 1?   Number    First sunday
      //         Sunday           0 (7)              1  (8-n)
      //         Monday           1                  7  (8-n)
      //         Tuesday          2                  6  (8-n)
      //         Wednesday        3                  5  (8-n)
      //         Thurdsay         4                  4  (8-n)
      //         Friday           5                  3  (8-n)
      //         Saturday         6                  2  (8-n)
      int firstSunday=8-firstDay;
      if(verbose){
        Serial.print("In November ");
        Serial.print(" day=");Serial.print(day);
        Serial.print(" firstSunday=");Serial.print(firstSunday);
      }
      if(day<firstSunday) {
        if(verbose)Serial.println("Before first Sunday, in DST");
        result=true;
      } else if(day>firstSunday) {
        if(verbose)Serial.println("After first Sunday, not DST");
        result=false;
      } else {
        result=(hour<2);
        if(verbose){Serial.print("On first Sunday, hour=");Serial.print(hour);Serial.print(", DST is");Serial.println(result?"on":"off");}
      }
    }
  }
  return result;
}

void toLocalTime() {
   m=gm;d=gd;y=gy;
   //Initial standard timezone correction. h is in 24h time to start with
   h=(gh-tz);n=gn;s=gs;
   if(h<0) {
     h+=24;
     d--; //Might result in d=0, but we don't care, DST code will still work and date is not displayed
   }
   //Now change h to 12 hour and account for DST
   bool thisDST=isDST(y,m,d,h);
   h=(h+(thisDST?1:0))%12;
}

