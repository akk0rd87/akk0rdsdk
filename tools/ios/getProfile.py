from pathlib import Path
import requests
import os
import re
import base64
import getToken

profile_save_path = os.environ['HOME'] + "/Library/MobileDevice/Provisioning Profiles"
Path(profile_save_path).mkdir(parents=True, exist_ok=True)

requestHeaders = getToken.getRequestHeaders()

def getProfile(app_bundle_name):
    URL = "https://api.appstoreconnect.apple.com/v1/bundleIds?filter[identifier]=" + app_bundle_name
    appBundleId = ""

    # READ THE APP BUNDLE ID BY BUNDLE_NAME
    r = requests.get(URL, headers=requestHeaders)
    data = r.json()
    for item in data["data"]:
        attrs = item['attributes']
        if(attrs['identifier'] == app_bundle_name) :
            appBundleId = item['id']

    print(f"[{app_bundle_name}]: Bundleid = {appBundleId}")

    # READ PROFILES BY APP BUNDLE ID
    URL = "https://api.appstoreconnect.apple.com/v1/bundleIds/" + appBundleId + "/profiles?fields[profiles]=profileState,bundleId,uuid,name,profileType,profileContent,expirationDate"
    r = requests.get(URL, headers=requestHeaders)
    data = r.json()
    # print(data)

    for item in data["data"]:
        attrs = item['attributes']
        if(attrs['profileState'] == 'ACTIVE' and attrs['profileType'] == 'IOS_APP_STORE' and attrs['name'] == 'AllApps') :
            print(f"Found profile for [{app_bundle_name}]: Bundleid = {appBundleId}")
            print(f"profile name : {attrs['name']}")
            print(f"ProfileID = {item['id']}")
            print(f"profile state: {attrs['profileState']}")
            print(f"profile type : {attrs['profileType']}")
            print(f"profile uuid : {attrs['uuid']}")
            print(f"profile expirationDate : {attrs['expirationDate']}")

            # save profile
            profile_filename = profile_save_path + "/" + attrs['uuid'] + '.mobileprovision'
            if os.path.exists(profile_filename):
                print(f"File {profile_filename} already exists")
            else :
                print(f"Save profile in {profile_filename}")
                with open(profile_filename, 'wb') as fw:
                    fw.write(base64.b64decode(attrs['profileContent']))

            # replace profile name in exportOptions.plist file
            optionsFilename = os.environ['EXPORT_OPTIONS_FILE']
            with open(optionsFilename, "r") as f:
                newText=f.read().replace('$(PROVISION_PROFILE_UUID)', attrs['name'])

            with open(optionsFilename, "w", newline='\n') as f:
                f.write(newText)

            return 0

    print(f"Did not find profiles for {app_bundle_name}")
    return 1

if (0 != getProfile(os.environ['APP_BUNDLE_ID'])) :
    getProfile('*')