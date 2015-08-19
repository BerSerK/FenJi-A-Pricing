import os, sys

def fix(filename):
    fp = open(filename);
    buf = fp.read()
    buf = buf.replace('\r\n', '\n');
    buf = buf.replace('\r', '\n');
    #print lines
    fp.close()
    fp = open(filename, 'w');
    fp.write(buf)
    fp.close()

if __name__ == "__main__":
    if len(sys.argv) == 2:
        fix( sys.argv[1])
    
        
