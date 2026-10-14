from gerbonara.excellon import ExcellonFile
from gerbonara.utils import PulldownSettings
try:
    from gerbonara.excellon import ExcellonSettings
    print("settings cls ok")
except Exception as e: print(e)
import inspect
print(inspect.signature(ExcellonFile.open))
print(inspect.signature(ExcellonFile.from_string))
