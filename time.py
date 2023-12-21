import datetime
import subprocess
import sys
import os

t = datetime.datetime.now()
before = t.strftime("%M:%S.%f")[:-3]
#before_list = before.split(':')
before_s = int(before.split(':')[1].split('.')[0])
before_ms = int(before.split(':')[1].split('.')[1])
#before_ms = int(before_ms[1])

subprocess.run([f'{sys.argv[1]}', f'{sys.argv[2]}'])

t = datetime.datetime.now()
after = t.strftime("%M:%S.%f")[:-3]
after_s = int(after.split(':')[1].split('.')[0])
after_ms = int(after.split(':')[1].split('.')[1])

print(f"\nStart: {before}\nEnd: {after}\nTotal: {after_s - before_s} S, {after_ms - before_ms} MS")
