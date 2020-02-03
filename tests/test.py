import _pyant
_pyant.set_debug(0)
a = _pyant.ANTUSBInterface()
b = _pyant.ANT(a)
b.init()
c = b.getChannel(0)
c.start(c.TYPE.TYPE_PAIR, 0x0000, True, True)
d = c.getDeviceList()[0]
e = d.getTsData()['HEARTRATE']
e.getValue()
