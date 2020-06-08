#include "gprmc.h"

//Hours the local time zone is behind UTC during standard time - positive for all US time zones
const int tz=7;
//Set to true if the location you are in uses DST - If so, code figures out if DST is in effect, if not, uses standard time year-round
const bool useDST=true;

bool GPRMC::expectDollar(char in) {if(in=='$') {state=&GPRMC::expectG     ;runChksum=0;}                            return false;}
bool GPRMC::expectG     (char in) {if(in=='G') {state=&GPRMC::expectP     ;return false;}state=&GPRMC::expectDollar;return false;}
bool GPRMC::expectP     (char in) {if(in=='P') {state=&GPRMC::expectR     ;return false;}state=&GPRMC::expectDollar;return false;}
bool GPRMC::expectR     (char in) {if(in=='R') {state=&GPRMC::expectM     ;return false;}state=&GPRMC::expectDollar;return false;}
bool GPRMC::expectM     (char in) {if(in=='M') {state=&GPRMC::expectC     ;return false;}state=&GPRMC::expectDollar;return false;}
bool GPRMC::expectC     (char in) {if(in=='C') {state=&GPRMC::expectComma1;return false;}state=&GPRMC::expectDollar;return false;}
bool GPRMC::expectComma1(char in) {if(in==',') {state=&GPRMC::expectHour0 ;return false;}state=&GPRMC::expectDollar;return false;}

bool GPRMC::expectHour0 (char in) {
  if(in>='0' && in<='9') {
    gh=(in-'0')*10;
    state=&GPRMC::expectHour1;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectHour1(char in) {
  if(in>='0' && in<='9') {
    gh+=(in-'0');
    state=&GPRMC::expectMinute0;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMinute0(char in) {
  if(in>='0' && in<='9') {
    gn=(in-'0')*10;
    state=&GPRMC::expectMinute1;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMinute1(char in) {
  if(in>='0' && in<='9') {
    gn+=(in-'0');
    state=&GPRMC::expectSecond0;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectSecond0(char in) {
  if((in>='0') && (in<='9')) {
    gs=(in-'0')*10;
    state=&GPRMC::expectSecond1;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectSecond1(char in) {
  if((in>='0') & (in<='9')) {
    gs+=(in-'0');
    state=&GPRMC::maybeSFraction;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::maybeSFraction(char in) {
  if(in==',') {
    state=&GPRMC::expectAV;
    return false;
  } else if(in=='.') {
    state=&GPRMC::expectSFraction;
    divisor=1;
    f=0;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectSFraction(char in) {
  if(in==',') {
    gs+=(f/divisor);
    state=&GPRMC::expectAV;
    return false;
  } else if((in>='0') & (in<='9')) {
    divisor*=10;
    f*=10;
    f+=(in-'0');
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectAV(char in) {
  if(in=='A') {
    gvalid=true;
    state=&GPRMC::expectComma3;
    return false;
  } else if(in=='V') {
    gvalid=false;
    state=&GPRMC::expectComma3;
    return false;
  } else if(in==',') {
    gvalid=false;
    state=&GPRMC::expectLatDegMin;
  }
  Serial.println("Problem with GPS time: expectAV");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectComma3(char in) {
    if(in==',') {
        state=&GPRMC::expectLatDegMin;
        glatdeg=0;
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectLatDegMin(char in) {
    if((in==',')|(in=='.')) {
        glatmin = glatdeg % 100;
        glatdeg = glatdeg / 100;
        f=0;
        divisor=1;
        state = (in==',')?&GPRMC::expectNS:&GPRMC::expectLatFrac;
        return false;
    } else if((in>='0') & (in<='9')) {
        glatdeg*=10;
        glatdeg+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectLatFrac(char in) {
    if(in==',') {
        glatmin+=(f/divisor);
        state=&GPRMC::expectNS;
        return false;
    } else if((in>='0') & (in<='9')) {
        divisor*=10;
        f*=10;
        f+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectNS(char in) {
    if(in==',') {
        state=&GPRMC::expectLonDegMin;
        return false;
    } else if(in=='N') {
        state=&GPRMC::expectComma5;
        return false;
    } else if(in=='S') {
        glatdeg=-glatdeg;
        glatmin=-glatmin;
        state=&GPRMC::expectComma5;
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectComma5(char in) {
    if(in==',') {
        state=&GPRMC::expectLonDegMin;
        glondeg=0;
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectLonDegMin(char in) {
    if((in==',')|(in=='.')) {
        glonmin = glondeg % 100;
        glondeg = glondeg / 100;
        f=0;
        divisor=1;
        state = (in==',')?&GPRMC::expectEW:&GPRMC::expectLonFrac;
        return false;
    } else if((in>='0') & (in<='9')) {
        glondeg*=10;
        glondeg+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectLonFrac(char in) {
    if(in==',') {
        glonmin+=(f/divisor);
        state=&GPRMC::expectEW;
        return false;
    } else if((in>='0') & (in<='9')) {
        divisor*=10;
        f*=10;
        f+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectEW(char in) {
    if(in==',') {
        gspd=0;
        state=&GPRMC::expectSpd;
        return false;
    } else if(in=='E') {
        state=&GPRMC::expectComma7;
        return false;
    } else if(in=='W') {
        glondeg=-glondeg;
        glonmin=-glonmin;
        state=&GPRMC::expectComma7;
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectComma7(char in) {
    if(in==',') {
        state=&GPRMC::expectSpd;
        gspd=0;
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectSpd(char in) {
    if((in==',')|(in=='.')) {
        f=0;
        divisor=1;
        state = (in==',')?&GPRMC::expectTrk:&GPRMC::expectSpdFrac;
        return false;
    } else if((in>='0') & (in<='9')) {
        gspd*=10;
        gspd+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectSpdFrac(char in) {
    if(in==',') {
        gspd+=(f/divisor);
        gtrk=0;
        state=&GPRMC::expectTrk;
        return false;
    } else if((in>='0') & (in<='9')) {
        divisor*=10;
        f*=10;
        f+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectTrk(char in) {
    if((in==',')|(in=='.')) {
        f=0;
        divisor=1;
        state = (in==',')?&GPRMC::expectDay0:&GPRMC::expectTrkFrac;
        return false;
    } else if((in>='0') & (in<='9')) {
        gspd*=10;
        gspd+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectTrkFrac(char in) {
    if(in==',') {
        gtrk+=(f/divisor);
        gd=0;
        state=&GPRMC::expectDay0;
        return false;
    } else if((in>='0') & (in<='9')) {
        divisor*=10;
        f*=10;
        f+=(in-'0');
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectDay0(char in) {
  if(in>='0' && in<='9') {
    gd=(in-'0')*10;
    state=&GPRMC::expectDay1;
    return false;
  }
  Serial.println("Problem with GPS time: Day0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectDay1(char in) {
  if(in>='0' && in<='9') {
    gd+=(in-'0');
    state=&GPRMC::expectMonth0;
    return false;
  }
  Serial.print("Problem with GPS time: Day1 ");
  Serial.println(in);
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMonth0(char in) {
  if(in>='0' && in<='9') {
    gm=(in-'0')*10;
    state=&GPRMC::expectMonth1;
    return false;
  }
  Serial.println("Problem with GPS time: Month0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMonth1(char in) {
  if((in>='0') && (in<='9')) {
    gm+=(in-'0');
    state=&GPRMC::expectYear0;
    return false;
  }
  Serial.println("Problem with GPS time: Month1");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectYear0(char in) {
  if((in>='0') && (in<='9')) {
    gy=(in-'0')*10+2000;
    state=&GPRMC::expectYear1;
    return false;
  }
  Serial.println("Problem with GPS time: Year0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectYear1(char in) {
    if ((in >= '0') && (in <= '9')) {
        gy += (in - '0');
        state = &GPRMC::waitForStar;
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::waitForStar(char in) {
    if (in == '*') {
        runChksum ^= in; //in was already accumulated, xor again to get it out
        state = &GPRMC::expectChksum0;
        return false;
    }
    return false;
}

bool GPRMC::expectChksum0(char in) {
    runChksum ^= in; //Back input character out of checksum
    if ((in == 13) || (in == 10)) {
        recChksum=runChksum; //No checksum recorded, so pretend that there was one and it matches
        state = &GPRMC::expectDollar;
        return validate();
    } else if ((in >= '0') & (in <= '9')) {
        recChksum = 16 * (in - '0');
        state = &GPRMC::expectChksum1;
        return false;
    } else if ((in >= 'A') & (in <= 'F')) {
        recChksum = 16 * (in - 'A' + 10);
        state = &GPRMC::expectChksum1;
        return false;
    } else if ((in >= 'a') & (in <= 'a')) {
        recChksum = 16 * (in - 'a' + 10);
        state = &GPRMC::expectChksum1;
        return false;
    }
    state=&GPRMC::expectDollar;
    return false;
}

bool GPRMC::expectChksum1(char in) {
    runChksum ^= in; //Back input character out of checksum
    state=&GPRMC::expectDollar;
    if ((in >= '0') & (in <= '9')) {
        recChksum +=  (in - '0');
        state = &GPRMC::expectChksum1;
        return validate();
    } else if ((in >= 'A') & (in <= 'F')) {
        recChksum +=  (in - 'A' + 10);
        state = &GPRMC::expectChksum1;
        return validate();
    } else if ((in >= 'a') & (in <= 'a')) {
        recChksum +=  (in - 'a' + 10);
        state = &GPRMC::expectChksum1;
        return validate();
    }
    return false;
}

bool GPRMC::validate() {
    //Presume that the GPS only produces well-formed sentences, and that therefore if the
    //checksum checks out, then it's good. If there is an error which makes the sentence
    //unparseable, it will be detected on the first invalid character and the state machine
    //will jump back to waiting for the first character of a sentence, therefore ignoring
    //the rest of an ill-formed sentence.
    wellformed=(runChksum==recChksum);
    if(wellformed) {
        valid=gvalid;
        count++;
        m=gm;d=gd;y=gy;
        h=gh;n=gn;s=gs;
        lat=glatdeg+glatmin/60;
        lon=glondeg+glonmin/60;
    }
    state=&GPRMC::expectDollar;
    return wellformed;
}
