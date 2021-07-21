import sys
import glob
from typing import List, Union
import serial
import struct
from csv import writer

cols = ["accel", "temperature", "pressure"]
# cols = ["accel_x", "accel_y", "accel_z", "altitude", "temperature", "pressure"]

def check_available_ports() -> List[str]:
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def select_port(available_ports: List[str]) -> str:
    if(len(available_ports) == 0):
        raise ConnectionError("no Arduino detected!")
    elif(len(available_ports) == 1):
        return available_ports[0]
    else:
        print()
        print("\nselect one of the following ports:")
        for i, p in enumerate(available_ports):
            print("  " + str(i + 1) + ": " + p)
        selected_index = int(input("> "))
        return available_ports[selected_index - 1]

def open_connection(serial_port: str) -> serial.Serial:
    serial_connection = serial.Serial(serial_port, baudrate=250000, timeout=2.0)
    print(f"\nconnected to {serial_port}\n")
    return serial_connection

def close_connection(serial_connection: serial.Serial):
    serial_connection.close()
    print("\nclosed connection\n")

def read_hex(file_name: str, serial_connection: serial.Serial) -> List[float]:
    serial_connection.write('a'.encode('ascii'))
    serial_connection.flush()
    serial_connection.readline()

    with open(file_name, "w+") as hex_file:
        numbers = []
        for i in range(512):
            line = None
            try:
                line = serial_connection.readline().strip().decode('ascii')
            except TimeoutError:
                print("\ntimed out\n")
                break
            
            hex_file.write(line + "\n")

            i2 = str(i).rjust(3, "0")
            print(f"{i2}\t: {line}")

            hex_codes = line.split(' ')
            hex_codes = list(map(lambda x: x.rjust(2, "0"), hex_codes))

            for j in range(0, len(hex_codes) - 3, 4):
                num = struct.unpack('!f', bytes.fromhex("".join(hex_codes[j:j+4])))[0]
                numbers.append(num)

    return numbers

def organize_data(numbers: List[float]) -> List[Union[List[str], List[float]]]:
    print()
    data = [ ["time"] + cols ]
    c = len(cols)
    t = 0
    for i in range(0, len(numbers) - c + 1, c):
        row = [t]
        t += 50
        for j in range(i, i + c):
            row.append(numbers[j])
        data.append(row)

        i2 = str(i // c).rjust(3, "0")
        row2 = " ".join(map(lambda x: str(x).rjust(25, " "), row))
        print(f"{i2}\t: {row2}")
    return data

def save_data(file_name: str, data: List[Union[List[str], List[float]]]):
    with open(file_name, "w+", newline='') as csv_file:
        csv_writer = writer(csv_file)
        csv_writer.writerows(data)

def main():
    available_ports = check_available_ports()
    serial_port = select_port(available_ports)
    serial_connection = open_connection(serial_port)

    numbers = read_hex("../Data/Raw/Latest Data.txt", serial_connection)
    data = organize_data(numbers)
    save_data("../Data/CSV/Latest Data.csv", data)
    
    close_connection(serial_connection)

main()