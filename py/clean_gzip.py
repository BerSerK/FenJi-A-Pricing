import os
def clean(dirname):
    fs = os.listdir(dirname)
    for f in fs:
        p = dirname +'/'+f
        if not os.path.isdir(p):
            if p.endswith('.gz.gz'):
                cmd = "gunzip %s"%p
                os.system( cmd )
            elif not p.endswith('.gz'):
                cmd = "gzip -f %s"%p
                os.system( cmd )
        else:
            print p
            clean(p)
                
fs = os.listdir('data')
for f in fs:
    clean('data/'+f) 
    
    
