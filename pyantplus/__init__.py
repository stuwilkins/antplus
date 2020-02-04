import _pyantplus
from _pyantplus import (ANT, ANTChannel, ANTDevice, ANTDeviceData,
                        ANTUSBInterface, ANTDeviceID) 

__all__ = ['_pyantplus']

from ._version import get_versions
__version__ = get_versions()['version']
del get_versions
