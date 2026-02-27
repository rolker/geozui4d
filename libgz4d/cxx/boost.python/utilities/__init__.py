from Time import *

def TimeString(self):
    import datetime
    return datetime.datetime.utcfromtimestamp(self).isoformat()

Time.__str__ = TimeString
