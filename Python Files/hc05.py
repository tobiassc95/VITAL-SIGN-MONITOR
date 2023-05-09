import bluetooth
import numpy as np

# def bluetoothScan():
#     print("Scanning for bluetooth devices: ")
#     devs = bluetooth.discover_devices(lookup_names=True)#, lookup_class=True)
#     print(len(devs), "devices found.")
#     for addr,name in devs:#,device_class in devs:
#         #print("\n")
#         print("Device Name: ", name)
#         print("Device MAC Address: ", addr)
#         #print("Device Class: ", device_class)
#         servs = bluetooth.find_service(address=addr)
#         if len(servs) <= 0:
#             print("No services found on", addr)
#         else:
#             print(len(servs), "services found on", addr)
#             for s in servs:
#                 print("\nService Name: ", s["name"])
#                 print("Host: ", s["host"])
#                 print("Description: ", s["description"])
#                 print("Provided By: ", s["provider"])
#                 print("Protocol: ", s["protocol"])
#                 print("channel/PSM: ", s["port"])
#                 print("svc classes: ", s["service-classes"])
#                 print("profiles: ", s["profiles"])
#                 print("service id: ", s["service-id"])
#     return()

# def bluetoothCn(dev_name=None, dev_addr=None):
#     if dev_name == None and dev_addr == None:
#         print("Scanning for bluetooth devices...")
#         devs = bluetooth.discover_devices(lookup_names=True)
#         print(len(devs), "devices found.")
#         for addr,name in devs:
#             print("\nDevice Name: ", name)
#             print("Device Address: ", addr)
#     elif dev_name != None and dev_addr == None:
#         print("Scanning for bluetooth device ", dev_name, "...")
#         devs = bluetooth.discover_devices(lookup_names=True)
#         for addr,name in devs:
#             if(dev_name == name):
#                 dev_addr = addr
#                 print("Device found.")
#                 #dev_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
#                 dev_port = 1
#                 print("\nConnecting to ", dev_name, "...")
#                 dev_sock.connect((dev_addr, dev_port))
#                 print("Connected")
#                 return
#             else:
#                 dev_addr = None
#         print("\n", dev_name, " not found.")
#         return   
#     else:
#         #dev_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
#         dev_port = 1
#         if dev_name != None:
#             print("\nConnecting to ", dev_name, "...")
#         else:
#             print("\nConnecting...")
#         dev_sock.connect((dev_addr, dev_port))
#         print("Connected")
#         return

def bluetoothScan(dev_name=None):
    print("Scanning for bluetooth devices: ")
    devs = bluetooth.discover_devices(lookup_names=True)
    if dev_name == None:
        print(len(devs), "devices found.")
    for addr,name in devs:
        if dev_name == None:
            print("\nDevice Name: ", name)
            print("Device Address: ", addr)
        if(dev_name == name):
            dev_addr = addr
        else:
            dev_addr = None
    if dev_addr == None and dev_name != None:
        print("\n", dev_name, " not found.")
        return None
    elif dev_addr != None and dev_name != None:
        print("\n", dev_name, " found.")
        print("Device Name: ", name)
        print("Device Address: ", addr)
        return dev_addr
    else:
        return None

#dev_addr = bluetoothScan(dev_name="MOD_HC05")
dev_addr = "00:18:E4:35:0F:63"
dev_port = 1
dev_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
print("\nConnecting...")
dev_sock.connect((dev_addr, dev_port))
print("Connected")

# while(True):
#     data = "OK"
#     dev_sock.send(data)
#     print("SENT: ", data)

#     data = dev_sock.recv(25)
#     data_ = dev_sock.recv(25)
#     #data__ = dev_sock.recv(60)
#     data = list(data) + list(data_)
#     #data = list(data) + list(data_) + list(data__)
#     print("RECEIVED: ", data)
#     data_ = ""
#     for i in data:
#         data_ = data_ + chr(i)
#     print("RECIEVED_: ", data_)

while(True):
    data = dev_sock.recv(25)
    data_ = dev_sock.recv(25)
    #data__ = dev_sock.recv(60)
    data = list(data) + list(data_)
    #data = list(data) + list(data_) + list(data__)
    print("RECIEVED: ", data)
    data_ = ""
    for i in data:
        data_ = data_ + chr(i)
    print("RECIEVED_: ", data_)

    data = "OK"
    dev_sock.send(data)
    print("SENT: ", data)