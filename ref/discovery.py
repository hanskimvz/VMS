"""
디바이스 검색 서비스

UPnP(SSDP), ONVIF(WS-Discovery), ARP를 통한 디바이스 검색을 제공합니다.
"""

import os
import re
from typing import List, Dict, Set
from infrastructure.network.upnp_client import UpnpClient
from infrastructure.network.onvif_client import OnvifClient
from utils.network_utils import is_online


class DeviceDiscoveryService:
    """
    디바이스 검색 서비스
    
    네트워크상의 IPN 디바이스를 검색합니다.
    """
    
    def __init__(self, interface_ip: str = None):
        self.upnp_client = UpnpClient()
        self.onvif_client = OnvifClient(interface_ip=interface_ip)
    
    def discover_by_ssdp(self, timeout: float = 2.0) -> List[Dict[str, str]]:
        """
        SSDP로 디바이스 검색
        
        Args:
            timeout: 검색 타임아웃 (초)
        
        Returns:
            list: 발견된 디바이스 리스트
        
        Examples:
            >>> service = DeviceDiscoveryService()
            >>> devices = service.discover_by_ssdp()
            >>> for dev in devices:
            ...     print(f"{dev['usn']} - {dev['location']}")
        """
        return self.upnp_client.ssdp_discover(timeout)

    def discover_by_onvif(self, timeout: float = 3.0) -> List[Dict[str, str]]:
        """
        ONVIF WS-Discovery로 디바이스 검색

        Args:
            timeout: 검색 타임아웃 (초)

        Returns:
            list: 발견된 디바이스 리스트
        """
        return self.onvif_client.ws_discovery_probe(timeout)
    
    def discover_by_arp(self) -> List[Dict[str, str]]:
        """
        ARP로 디바이스 검색
        
        Returns:
            list: 발견된 디바이스 리스트
        """
        devices = []
        locations: Set[str] = set()
        
        # ARP 명령
        if os.name == 'nt':  # Windows
            cmd = 'arp -a |findstr "00-13-2"'
        else:  # Linux/Mac
            cmd = "arp -n |grep 00:13:2"
        
        # ARP 테이블 조회
        arp_regex = re.compile(r"([0-9.]+)(\s+)([\w:]+)(.+)", re.IGNORECASE)
        result = os.popen(cmd).read()
        result = result.replace("ether", "").replace("-", "").replace(":", "")
        
        lines = result.splitlines()
        for line in lines:
            match = arp_regex.search(line)
            if not match:
                continue
            
            location = match.group(1)
            mac = match.group(3).upper()
            
            # 온라인 확인
            if not is_online(location):
                continue
            
            if location in locations:
                continue
            
            # UPnP 정보 조회 시도
            try_ports = [80, 49152, 49153]
            try_pages = ['upnpdevicedesc.xml', 'DigitalSecurityCamera1.xml']
            
            for port in try_ports:
                for page in try_pages:
                    if location in locations:
                        break
                    
                    url = f'http://{location}:{port}/{page}'
                    info = self.upnp_client.get_device_info(url, timeout=1.0)
                    
                    if info:
                        devices.append({
                            'usn': info.get('usn', ''),
                            'url': url,
                            'location': location,
                            'mac': mac or info.get('mac', ''),
                            'model': info.get('model', ''),
                            'brand': info.get('brand', '')
                        })
                        locations.add(location)
        
        return devices
    
    def discover_all(self, timeout: float = 2.0) -> List[Dict[str, str]]:
        """
        모든 방법으로 디바이스 검색
        
        SSDP → ONVIF → ARP 순으로 시도하며 결과를 병합합니다.
        
        Args:
            timeout: 검색 타임아웃 (초)
        
        Returns:
            list: 발견된 디바이스 리스트
        
        Examples:
            >>> service = DeviceDiscoveryService()
            >>> devices = service.discover_all()
            >>> print(f"Found {len(devices)} devices")
        """
        devices: List[Dict[str, str]] = []
        seen_ips: Set[str] = set()

        def _merge(found: List[Dict[str, str]]) -> None:
            for dev in found:
                ip = dev.get("location", "")
                if not ip:
                    continue
                if ip in seen_ips:
                    continue
                seen_ips.add(ip)
                devices.append(dev)

        _merge(self.discover_by_ssdp(timeout))
        _merge(self.discover_by_onvif(timeout))
        if not devices:
            _merge(self.discover_by_arp())

        return devices
    
    def find_device_by_ip(self, ip: str) -> Dict[str, str]:
        """
        IP로 특정 디바이스 검색
        
        Args:
            ip: 디바이스 IP 주소
        
        Returns:
            dict: 디바이스 정보 또는 빈 딕셔너리
        """
        if not is_online(ip):
            return {}
        
        # UPnP 정보 조회 시도
        try_ports = [80, 49152, 49153]
        try_pages = ['upnpdevicedesc.xml', 'DigitalSecurityCamera1.xml']
        
        for port in try_ports:
            for page in try_pages:
                url = f'http://{ip}:{port}/{page}'
                info = self.upnp_client.get_device_info(url, timeout=1.0)
                
                if info:
                    return {
                        'usn': info.get('usn', ''),
                        'url': url,
                        'location': ip,
                        'mac': info.get('mac', ''),
                        'model': info.get('model', ''),
                        'brand': info.get('brand', '')
                    }
        
        return {}

