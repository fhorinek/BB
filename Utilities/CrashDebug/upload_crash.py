
#!/usr/bin/env python3

import platform
import subprocess
import argparse
import pathlib
import re
from datetime import datetime
import uuid
import requests
import json
import base64
import yaml
import hashlib

# Arguments

parser = argparse.ArgumentParser(description='Upload Strato crash to AppCenter (arm-none-eabi-gdb needs to be available in the PATH)')
parser.add_argument('--secret', type=str,
                    help='AppCenter secret')
parser.add_argument('--elf', type=pathlib.Path,
                    help='path to the corresponding elf file')
parser.add_argument('crash_path', type=pathlib.Path,
                    help='path to the extracted Strato crash_report folder (with crash & config inside)')
args = parser.parse_args()

# Extract stack trace

crashdebug_path = "lin64/CrashDebug"
if platform.system() == "Darwin":
    crashdebug_path = "osx64/CrashDebug"

cmd = ( f'arm-none-eabi-gdb --batch --quiet {args.elf}'
        f' -ex "set target-charset ASCII"'
        f' -ex "target remote | {crashdebug_path} --elf {args.elf} --dump {args.crash_path}/crash/dump.bin"'
        f' -ex "set print pretty on"'
        f' -ex "bt full"'
        f' -ex "quit"')

process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
output, error = process.communicate()
stack_trace = output.decode("utf-8")

# Cleanup stack trace

end_regex = re.compile(r'Backtrace stopped(.|\n)*')
stack_trace = end_regex.sub('', stack_trace)

# Extract frames

frame_regex = re.compile(r'#\d+\s+(0x[a-e0-9]+) in (\S*).*at ([^:]*):(\d+)')
def map_frame(frame):
    return {
        'address': frame[0],
        'methodName': frame[1],
        'fileName': frame[2],
        'lineNumber': int(frame[3])
    }
frames = list(map(map_frame, frame_regex.findall(stack_trace)))

code_regex = re.compile(r'^\d+\s+(.*)', re.MULTILINE)
code = code_regex.search(stack_trace)
if code is not None:
    start_frame = dict(frames[0])
    start_frame['code'] = code.group(1)
    frames.insert(0, start_frame)

# Extract info

info_file_path = f'{args.crash_path}/crash/info.yaml'
info_file = yaml.safe_load(open(info_file_path, "r").read())

message = ', '.join(info_file['fault']['reasons'])
if 'message' in info_file:
    info_message = info_file['message']
    message = f'{info_message} ({message})'

exception = {
    'type': info_file['fault']['status'],
    'message': message,
    'frames': frames,
}

hardware_revision = info_file['hardware_revision']
device = {
    'appVersion': info_file['firmware_version'],
    'appBuild': info_file['firmware_version'],
    'sdkName': 'appcenter.custom',
    'sdkVersion': '0.0.0',
    'osName': '',
    'osVersion': '0.0.0',
    'model': f'strato-{hardware_revision}',
    'locale': 'en-US'
}

# Assemble error log

error_log = {
    'type': 'managedError',
    'device': device,
    'timestamp': datetime.utcnow().isoformat(timespec='seconds') + 'Z',
    'appLaunchTimestamp': info_file['timestamp'],
    'id': str(uuid.uuid4()),
    'processId': 0,
    'processName': 'main',
    'fatal': True,
    'exception': exception
}

all_logs = [error_log]

# Add attachments

def encode_file(path):
    return base64.b64encode(open(path, "rb").read()).decode('utf-8')

def file_attachment_log(name, data, content_type = 'text/plain'):
    return {
        'type': 'errorAttachment',
        'device': device,
        'id': str(uuid.uuid4()),
        'errorId': error_log['id'],
        'contentType': content_type,
        'data': data,
        'fileName': name
    }

all_logs.append(file_attachment_log('info.txt', encode_file(info_file_path)))
all_logs.append(file_attachment_log('files.txt', encode_file(f'{args.crash_path}/crash/files.txt')))
all_logs.append(file_attachment_log('trace.txt', base64.b64encode(stack_trace.encode('utf-8')).decode('utf-8')))
all_logs.append(file_attachment_log('dump.bin', encode_file(f'{args.crash_path}/crash/dump.bin'), 'application/octet-stream'))


# Upload

# Create UUID based on 12 byte device ID
md5 = hashlib.md5()
md5.update(info_file['serial_number'].encode('utf-8'))
install_id = str(uuid.UUID(md5.hexdigest()))

url = 'https://in.appcenter.ms/logs?Api-Version=1.0.0'
headers = {
    'Content-Type': 'application/json',
    'app-secret': args.secret,
    'install-id': install_id
}
data = json.dumps({ 'logs': all_logs })

# print(data)
response = requests.post(url, headers=headers, data=data)
print(f'Response: [{response.status_code}] {response.text}')
