
from wsdiscovery.discovery import ThreadedWSDiscovery as WSDiscovery
from wsdiscovery import Scope
import re

def display(any_list):
    for item in any_list:
        print(item)

def fetch_devices():
    wsd = WSDiscovery()
    scope1 = Scope("onvif://www.onvif.org/Profile")
    # scope1 = Scope("onvif://www.onvif.org/Model")
    wsd.start()
    services = wsd.searchServices(scopes=[scope1])
    ipaddresses = []
    for service in services:
        for s in service:
            print (s)
    #filter those devices that dont have ONVIF service
        # ipaddress = re.search('(\d+|\.)+', str(service.getXAddrs()[0])).group(0)
        # ipaddresses.append(ipaddress)
        # print(display(service.getScopes()))
        # print(service.getEPR() + ":" + service.getXAddrs()[0])
        # print('----------END')

    print(f'\nnumber of devices detected: {len(services)}')
    wsd.stop()
    return ipaddresses

if __name__ == "__main__":
    onvif_devices_IPs = fetch_devices()
    # display(onvif_devices_IPs)