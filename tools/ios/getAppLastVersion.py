from datetime import datetime, timedelta
from time import time, mktime
import jwt
import os
import requests
import re

dt = datetime.now() + timedelta(minutes=5)

headers = {
    "alg": "ES256",
    "kid": os.environ['API_KEY_ID'],
    "typ": "JWT",
}

payload = {
    "iss": os.environ['API_ISSUER'],
    "iat": int(time()),
    "exp": int(mktime(dt.timetuple())),
    "aud": "appstoreconnect-v1",
}


with open(os.environ['API_KEY_FILE'], "rb") as fh: # Add your file
    signing_key = fh.read()

access_token =  jwt.encode(payload, signing_key, algorithm="ES256", headers=headers)

URL = "https://api.appstoreconnect.apple.com/v1/builds?filter[app]=" + os.environ['APP_APPLE_ID'] + "&sort=-uploadedDate&limit=15&fields[builds]=version"

r = requests.get(URL, headers={'Authorization': 'Bearer {}'.format(access_token)})
data = r.json()
print(data)

last_version = 0

for item in data["data"]:
  attrs = item['attributes']
  print(attrs['version'])
  current = max(map(int, re.findall(r'\d+$', attrs['version'] )))
  if (last_version < current):
     last_version = current


print(f"last_version= {last_version}")

new_version = last_version + 1
print(f"new_version= {new_version}")

f = open(os.environ['NEW_VERSION_FILE'], "w")
f.write("#!/bin/bash\n")
f.write(f"export NEW_IOS_BUNDLE_VERSION={new_version}")
f.close()