import pyantplus
import time
pyantplus._pyantplus.set_debug(1)
a = pyantplus.ANTUSBInterface()
b = pyantplus.ANT(a)
b.init()
c = b.getChannel(0)
c.start(c.TYPE.TYPE_PAIR)
time.sleep(10)
d = c.getDeviceList()[0]
e = d.getTsData()['HEARTRATE']
e.getValue()
