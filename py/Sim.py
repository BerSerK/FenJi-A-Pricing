import os, sys
import datetime

from config import get_config

def next_friday( d ):
    if d.weekday() <= 4:
        time_delta = datetime.timedelta( 4 - d.weekday() )
    else:
        time_delta = datetime.timedelta( 11 - d.weekday())
    return d + time_delta

START_DATE = datetime.date(2015,6,16);
END_DATE = datetime.date(2015,10,5);

def get_dates():
    data_dir = 'doc/config2014/nav/'
    fs = os.listdir(data_dir)
    dates = []
    for f in fs:
        ds = f.split('_')[1]
        ds = ds[0:8]
        d = datetime.date(int(ds[0:4]), int(ds[4:6]), int(ds[6:8]))
        dates.append(d)
    dates.sort()
    return dates

def sim_day( ds ):
    get_config( ds )
    cmd = "./bin/Main config/config%s.txt"%ds
    os.system( cmd )
    cmdgzip = "gzip -r data/S%s"%ds
    os.system( cmdgzip )
    
    
def sim_friday():
    dates = get_dates()
    d = dates[ 0 ];
    NextFriday = next_friday( d );
    for i in range(len(dates)):
        d = dates[ i ]
        if d < START_DATE or d > END_DATE:
            continue
        ds = d.strftime("%Y%m%d")
        #print ds
        #if d.weekday() == 4:
        if i == len(dates) - 1:
            next_day = d + datetime.timedelta(7)
        else:
            next_day = dates[ i + 1]
        if next_day > NextFriday:
            NextFriday = next_friday( next_day );
            sim_day( ds )

def sim_all():
    dates = get_dates()
    d = dates[ 0 ];
    for i in range(len(dates)):
        d = dates[ i ]
        if d < START_DATE or d > END_DATE:
            continue
        ds = d.strftime("%Y%m%d")
        sim_day( ds )

if __name__ == "__main__":
    sim_all()
