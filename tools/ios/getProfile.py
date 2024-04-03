from datetime import datetime, timedelta
from time import time, mktime
import jwt
import os
import requests
import re
import base64

profile_save_path = os.environ['HOME'] + "/Library/MobileDevice/Provisioning Profiles"
app_bundle_name = os.environ['APP_BUNDLE_ID']

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

requestHeaders = {'Authorization': 'Bearer {}'.format(access_token)}

URL = "https://api.appstoreconnect.apple.com/v1/bundleIds?filter[identifier]=" + app_bundle_name
appBundleId = ""

# READ THE APP BUNDLE ID BY BUNDLE_NAME
r = requests.get(URL, headers=requestHeaders)
data = r.json()
for item in data["data"]:
    attrs = item['attributes']
    if(attrs['identifier'] == app_bundle_name) :
        appBundleId = item['id']

print(appBundleId)

# READ PROFILES BY APP BUNDLE ID
URL = "https://api.appstoreconnect.apple.com/v1/bundleIds/" + appBundleId + "/profiles?fields[profiles]=profileState,bundleId,uuid,name,profileType,profileContent"

r = requests.get(URL, headers=requestHeaders)
data = r.json()
# print(data)

for item in data["data"]:
    attrs = item['attributes']
    if(attrs['profileState'] == 'ACTIVE' and attrs['profileType'] == 'IOS_APP_STORE') :
        print(f"ProfileID = {item['id']}")
        print(attrs['profileState'])
        print(attrs['profileType'])
        print(attrs['uuid'])
        with open(profile_save_path + "/" + attrs['uuid'] + '.mobileprovision', 'wb') as fw:
            fw.write(base64.b64decode(attrs['profileContent']))