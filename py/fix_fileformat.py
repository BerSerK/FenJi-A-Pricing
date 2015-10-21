import os, sys

def fix_endline(filename):
    fp = open(filename);
    buf = fp.read()
    buf = buf.replace('\r\n', '\n');
    buf = buf.replace('\r', '\n');
    #print lines
    fp.close()
    fp = open(filename, 'w');
    fp.write(buf)
    fp.close()

def convert_encoding(from_file):
    s = from_file.split("/");
    tofile = from_file.replace( s[-1], 'utf8_'+s[-1])
    cmd = "iconv -f gbk -t utf-8 %s > %s"%(from_file, tofile)
    print cmd
    os.system( cmd )

def fix_dir( dir ):
    fs = os.listdir( dir )
    for f in fs:
        fn = dir + "/" + f
        fix_endline( fn )
        if not f.startswith("utf"):
            convert_encoding( fn )

if __name__ == "__main__":
    if len(sys.argv) == 2:
        fix_dir( sys.argv[1] )
