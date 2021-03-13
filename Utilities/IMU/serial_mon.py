import serial
soc = serial.Serial("/dev/ttyACM0", 921600, timeout=None)

while True:
    values = soc.read_until().decode("ascii").split(" ")
    print(values)
