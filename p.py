import datetime
import subprocess
import sys
import os

t = datetime.datetime.now()
before = t.strftime("%M:%S.%f")[:-3]
before_list = before.split(':')
before_ms = before_list[1].split('.')
before_ms = int(before_ms[1])

subprocess.run([f'{sys.argv[1]}', f'{sys.argv[2]}'])

t = datetime.datetime.now()
after = t.strftime("%M:%S.%f")[:-3]
after_list = after.split(':')
after_ms = after_list[1].split('.')
after_ms = int(after_ms[1])

print(f"\nStart: {before}\nEnd: {after}\nTotal: {after_ms - before_ms} MS")
