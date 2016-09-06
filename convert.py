import sys
if len(sys.argv) < 2:
    print("%s path/to/result.py" % sys.argv[0])
sys.path.insert(0, sys.argv[1])
import numpy
from result import test_res, train_res
template = """{
x: %s,
y: %s,
mode: "markers", 
type: "scatter",
text: %s,
marker: {size: 6}
}"""
def convert(m):
    labels = m.keys()
    x = [m[k]['precision'] for k in labels]
    y = [m[k]['recall'] for k in labels]
    return  template % (x,y,labels)

from os import path as p
path = sys.argv[1]
while path.endswith("/"):
    path = path[:-1]
basename = p.basename(path)
basename =  basename.replace(" ", "_")
outname = basename+".js"

f = open(outname,"w")
f.write("%s_test = %s;\n" % (basename,convert(test_res)))
f.write("%s_train = %s;\n" % (basename,convert(train_res)))
f.close()
