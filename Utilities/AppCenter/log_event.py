#!/usr/bin/env python3

import argparse
import hashlib
import uuid
from datetime import datetime
import json
import requests

parser = argparse.ArgumentParser(
    description='Send an analytics event to AppCenter')
parser.add_argument('--secret', type=str,
                    help='AppCenter secret')
parser.add_argument('--serial-number', type=str,
                    help='The serial number of the Strato device')
parser.add_argument('--firmware-version', type=str,
                    help='The firmware version of the Strato device')
parser.add_argument('--hardware-revision', type=str,
                    help='The hardware revision of the Strato device')
parser.add_argument('--event', type=str,
                    help='The event name', default='update-check')
args = parser.parse_args()

if args.secret is None:
    parser.error('Missing AppCenter secret, pass via --secret')

if args.firmware_version is None:
    parser.error('Missing firmware version, pass via --firmware-version')

if args.hardware_revision is not None:
    model = f"Strato ({args.hardware_revision})"
else:
    model = "Strato"

log_entry = {
    "sid": str(uuid.uuid4()),
    "timestamp": datetime.utcnow().isoformat(timespec='seconds') + 'Z',
    "device": {
        "appVersion": args.firmware_version,
        "appBuild": args.firmware_version,
        "sdkName": "appcenter.custom",
        "sdkVersion": "0.0.0",
        "osName": "None",
        "osVersion": "0.0.0",
        "model": model,
        "locale": "en-US",
        "timeZoneOffset": 0
    }
}

if args.serial_number is not None:
    log_entry["userId"] = args.serial_number

payload = {
    "logs": [
        {
            "id": str(uuid.uuid4()),
            "type": "startSession",
            **log_entry
        },
        {
            "id": str(uuid.uuid4()),
            "type": "event",
            "name": args.event,
            **log_entry        
        }
    ]
}

# Create UUID based on 12 byte device ID (if available)
if args.serial_number is not None:
    md5 = hashlib.md5()
    md5.update(args.serial_number.encode('utf-8'))
    install_id = str(uuid.UUID(md5.hexdigest()))
else:
    install_id = str(uuid.uuid4())

url = 'https://in.appcenter.ms/logs?Api-Version=1.0.0'
headers = {
    'Content-Type': 'application/json',
    'app-secret': args.secret,
    'install-id': install_id
}
data = json.dumps(payload)

# print(headers)
# print(data)
response = requests.post(url, headers=headers, data=data)
print(f'Response: [{response.status_code}] {response.text}')
