from datetime import datetime, timedelta
from time import time, mktime
import jwt
import os

def getToken():
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

    access_token = jwt.encode(payload, signing_key, algorithm="ES256", headers=headers)
    return access_token

def getRequestHeaders():
    return {'Authorization': 'Bearer {}'.format(getToken())}