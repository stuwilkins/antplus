import pyantplus
import time
pyantplus._pyantplus.set_debug(0)
a = pyantplus.ANTUSBInterface()
b = pyantplus.ANT(a)
b.init()
c = b.getChannel(0)
c.start(c.TYPE.TYPE_PAIR, 0x0000, True, True)
time.sleep(10)
d = c.getDeviceList()[0]
e = d.getTsData()['HEARTRATE']
e.getValue()
