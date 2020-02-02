import _pyant

__all__ = ['_pyant']

from ._version import get_versions
__version__ = get_versions()['version']
del get_versions
