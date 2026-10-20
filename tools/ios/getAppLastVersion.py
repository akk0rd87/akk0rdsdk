from datetime import datetime, timedelta
from time import time, mktime
import os
import requests
import re
import sys
import getToken


URL = "https://api.appstoreconnect.apple.com/v1/builds?filter[app]=" + os.environ['APP_APPLE_ID'] + "&sort=-uploadedDate&limit=15&fields[builds]=version"

r = requests.get(URL, headers=getToken.getRequestHeaders())
if not r.ok:
  sys.exit(f"App Store Connect: HTTP {r.status_code} {r.text}")
data = r.json()
print(data)

last_version = 0

for item in data["data"]:
  attrs = item['attributes']
  print(attrs['version'])
  digits = re.findall(r'\d+$', attrs['version'])
  if not digits:
    continue
  current = int(digits[0])
  if (last_version < current):
     last_version = current


print(f"last_version={last_version}")

new_version = last_version + int(os.environ.get('VERSION_INCREMENT') or 1)
print(f"new_version={new_version}")

f = open(os.environ['NEW_VERSION_FILE'], "w")
f.write("#!/bin/bash\n")
f.write(f"export NEW_IOS_BUNDLE_VERSION={new_version}")
f.close()