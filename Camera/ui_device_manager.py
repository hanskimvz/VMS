# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'device_managerwBEciR.ui'
##
## Created by: Qt User Interface Compiler version 5.15.2
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from essential import *

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(676, 537)
        MainWindow.setUnifiedTitleAndToolBarOnMac(False)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.main_tab = QTabWidget(self.centralwidget)
        self.main_tab.setObjectName(u"main_tab")
        self.main_tab.setGeometry(QRect(0, 0, 680, 541))
        self.network_main = QWidget()
        self.network_main.setObjectName(u"network_main")
        self.network_tab = QTabWidget(self.network_main)
        self.network_tab.setObjectName(u"network_tab")
        self.network_tab.setGeometry(QRect(10, 10, 651, 461))
        self.network_ethernet = QWidget()
        self.network_ethernet.setObjectName(u"network_ethernet")
        self.gridLayoutWidget = QWidget(self.network_ethernet)
        self.gridLayoutWidget.setObjectName(u"gridLayoutWidget")
        self.gridLayoutWidget.setGeometry(QRect(70, 30, 481, 221))
        self.network_status_grid = QGridLayout(self.gridLayoutWidget)
        self.network_status_grid.setObjectName(u"network_status_grid")
        self.network_status_grid.setContentsMargins(0, 0, 0, 0)
        self.label_dhcp = QLabel(self.gridLayoutWidget)
        self.label_dhcp.setObjectName(u"label_dhcp")

        self.network_status_grid.addWidget(self.label_dhcp, 1, 0, 1, 1)

        self.label_gateway = QLabel(self.gridLayoutWidget)
        self.label_gateway.setObjectName(u"label_gateway")

        self.network_status_grid.addWidget(self.label_gateway, 4, 0, 1, 1)

        self.label_mac = QLabel(self.gridLayoutWidget)
        self.label_mac.setObjectName(u"label_mac")

        self.network_status_grid.addWidget(self.label_mac, 0, 0, 1, 1)

        self.label_ip_address = QLabel(self.gridLayoutWidget)
        self.label_ip_address.setObjectName(u"label_ip_address")

        self.network_status_grid.addWidget(self.label_ip_address, 2, 0, 1, 1)

        self.label_subnetmask = QLabel(self.gridLayoutWidget)
        self.label_subnetmask.setObjectName(u"label_subnetmask")

        self.network_status_grid.addWidget(self.label_subnetmask, 3, 0, 1, 1)

        self.label_dns2 = QLabel(self.gridLayoutWidget)
        self.label_dns2.setObjectName(u"label_dns2")

        self.network_status_grid.addWidget(self.label_dns2, 6, 0, 1, 1)

        self.label_ip_self_adaption = QLabel(self.gridLayoutWidget)
        self.label_ip_self_adaption.setObjectName(u"label_ip_self_adaption")

        self.network_status_grid.addWidget(self.label_ip_self_adaption, 7, 0, 1, 1)

        self.label_dns1 = QLabel(self.gridLayoutWidget)
        self.label_dns1.setObjectName(u"label_dns1")

        self.network_status_grid.addWidget(self.label_dns1, 5, 0, 1, 1)

        self.radio_ip_self_yes = QRadioButton(self.gridLayoutWidget)
        self.network_ethernet_ipself_btngrp = QButtonGroup(MainWindow)
        self.network_ethernet_ipself_btngrp.setObjectName(u"network_ethernet_ipself_btngrp")
        self.network_ethernet_ipself_btngrp.addButton(self.radio_ip_self_yes)
        self.radio_ip_self_yes.setObjectName(u"radio_ip_self_yes")
        self.radio_ip_self_yes.setChecked(True)

        self.network_status_grid.addWidget(self.radio_ip_self_yes, 7, 2, 1, 1)

        self.radio_ip_self_no = QRadioButton(self.gridLayoutWidget)
        self.network_ethernet_ipself_btngrp.addButton(self.radio_ip_self_no)
        self.radio_ip_self_no.setObjectName(u"radio_ip_self_no")
        self.radio_ip_self_no.setAutoExclusive(True)

        self.network_status_grid.addWidget(self.radio_ip_self_no, 7, 3, 1, 1)

        self.mac_address = QLineEdit(self.gridLayoutWidget)
        self.mac_address.setObjectName(u"mac_address")
        self.mac_address.setReadOnly(True)

        self.network_status_grid.addWidget(self.mac_address, 0, 2, 1, 2)

        self.ipv4_addr = QLineEdit(self.gridLayoutWidget)
        self.ipv4_addr.setObjectName(u"ipv4_addr")

        self.network_status_grid.addWidget(self.ipv4_addr, 2, 2, 1, 2)

        self.ipv4_mask = QLineEdit(self.gridLayoutWidget)
        self.ipv4_mask.setObjectName(u"ipv4_mask")

        self.network_status_grid.addWidget(self.ipv4_mask, 3, 2, 1, 2)

        self.ipv4_gateway = QLineEdit(self.gridLayoutWidget)
        self.ipv4_gateway.setObjectName(u"ipv4_gateway")

        self.network_status_grid.addWidget(self.ipv4_gateway, 4, 2, 1, 2)

        self.dns1 = QLineEdit(self.gridLayoutWidget)
        self.dns1.setObjectName(u"dns1")

        self.network_status_grid.addWidget(self.dns1, 5, 2, 1, 2)

        self.dns2 = QLineEdit(self.gridLayoutWidget)
        self.dns2.setObjectName(u"dns2")

        self.network_status_grid.addWidget(self.dns2, 6, 2, 1, 2)

        self.network_ethernet_dhcp_yes = QRadioButton(self.gridLayoutWidget)
        self.network_ethernet_dhcp_btngrp = QButtonGroup(MainWindow)
        self.network_ethernet_dhcp_btngrp.setObjectName(u"network_ethernet_dhcp_btngrp")
        self.network_ethernet_dhcp_btngrp.addButton(self.network_ethernet_dhcp_yes)
        self.network_ethernet_dhcp_yes.setObjectName(u"network_ethernet_dhcp_yes")

        self.network_status_grid.addWidget(self.network_ethernet_dhcp_yes, 1, 2, 1, 1)

        self.network_ethernet_dhcp_no = QRadioButton(self.gridLayoutWidget)
        self.network_ethernet_dhcp_btngrp.addButton(self.network_ethernet_dhcp_no)
        self.network_ethernet_dhcp_no.setObjectName(u"network_ethernet_dhcp_no")

        self.network_status_grid.addWidget(self.network_ethernet_dhcp_no, 1, 3, 1, 1)

        self.network_tab.addTab(self.network_ethernet, "")
        self.network_utp = QWidget()
        self.network_utp.setObjectName(u"network_utp")
        self.gridLayoutWidget_3 = QWidget(self.network_utp)
        self.gridLayoutWidget_3.setObjectName(u"gridLayoutWidget_3")
        self.gridLayoutWidget_3.setGeometry(QRect(90, 70, 481, 151))
        self.udp_grid = QGridLayout(self.gridLayoutWidget_3)
        self.udp_grid.setObjectName(u"udp_grid")
        self.udp_grid.setContentsMargins(0, 0, 0, 0)
        self.label_multicast_play_address = QLabel(self.gridLayoutWidget_3)
        self.label_multicast_play_address.setObjectName(u"label_multicast_play_address")

        self.udp_grid.addWidget(self.label_multicast_play_address, 4, 0, 1, 1)

        self.label_muticast_ip = QLabel(self.gridLayoutWidget_3)
        self.label_muticast_ip.setObjectName(u"label_muticast_ip")

        self.udp_grid.addWidget(self.label_muticast_ip, 2, 0, 1, 1)

        self.label_multicast_port = QLabel(self.gridLayoutWidget_3)
        self.label_multicast_port.setObjectName(u"label_multicast_port")

        self.udp_grid.addWidget(self.label_multicast_port, 3, 0, 1, 1)

        self.network_ethernet_udp_yes = QRadioButton(self.gridLayoutWidget_3)
        self.network_udp_btngrp = QButtonGroup(MainWindow)
        self.network_udp_btngrp.setObjectName(u"network_udp_btngrp")
        self.network_udp_btngrp.addButton(self.network_ethernet_udp_yes)
        self.network_ethernet_udp_yes.setObjectName(u"network_ethernet_udp_yes")

        self.udp_grid.addWidget(self.network_ethernet_udp_yes, 0, 1, 1, 1)

        self.label_videostream = QLabel(self.gridLayoutWidget_3)
        self.label_videostream.setObjectName(u"label_videostream")

        self.udp_grid.addWidget(self.label_videostream, 1, 0, 1, 1)

        self.label_open_multitask = QLabel(self.gridLayoutWidget_3)
        self.label_open_multitask.setObjectName(u"label_open_multitask")

        self.udp_grid.addWidget(self.label_open_multitask, 0, 0, 1, 1)

        self.network_ethernet_udp_no = QRadioButton(self.gridLayoutWidget_3)
        self.network_udp_btngrp.addButton(self.network_ethernet_udp_no)
        self.network_ethernet_udp_no.setObjectName(u"network_ethernet_udp_no")

        self.udp_grid.addWidget(self.network_ethernet_udp_no, 0, 2, 1, 1)

        self.network_udp_stream = QComboBox(self.gridLayoutWidget_3)
        self.network_udp_stream.addItem("")
        self.network_udp_stream.addItem("")
        self.network_udp_stream.setObjectName(u"network_udp_stream")

        self.udp_grid.addWidget(self.network_udp_stream, 1, 1, 1, 2)

        self.network_multicast_ip = QLineEdit(self.gridLayoutWidget_3)
        self.network_multicast_ip.setObjectName(u"network_multicast_ip")

        self.udp_grid.addWidget(self.network_multicast_ip, 2, 1, 1, 2)

        self.upd_multicast_play_address = QLineEdit(self.gridLayoutWidget_3)
        self.upd_multicast_play_address.setObjectName(u"upd_multicast_play_address")
        self.upd_multicast_play_address.setReadOnly(True)

        self.udp_grid.addWidget(self.upd_multicast_play_address, 4, 1, 1, 2)

        self.udp_port = QSpinBox(self.gridLayoutWidget_3)
        self.udp_port.setObjectName(u"udp_port")
        self.udp_port.setMaximum(65535)

        self.udp_grid.addWidget(self.udp_port, 3, 1, 1, 2)

        self.network_tab.addTab(self.network_utp, "")
        self.network_rtmp = QWidget()
        self.network_rtmp.setObjectName(u"network_rtmp")
        self.gridLayoutWidget_4 = QWidget(self.network_rtmp)
        self.gridLayoutWidget_4.setObjectName(u"gridLayoutWidget_4")
        self.gridLayoutWidget_4.setGeometry(QRect(90, 40, 421, 241))
        self.rtmp_grid = QGridLayout(self.gridLayoutWidget_4)
        self.rtmp_grid.setObjectName(u"rtmp_grid")
        self.rtmp_grid.setContentsMargins(0, 0, 0, 0)
        self.label_rtmp_play_address = QLabel(self.gridLayoutWidget_4)
        self.label_rtmp_play_address.setObjectName(u"label_rtmp_play_address")

        self.rtmp_grid.addWidget(self.label_rtmp_play_address, 6, 0, 1, 1)

        self.app_name = QLineEdit(self.gridLayoutWidget_4)
        self.app_name.setObjectName(u"app_name")

        self.rtmp_grid.addWidget(self.app_name, 4, 1, 1, 2)

        self.rtmp_play_address = QLineEdit(self.gridLayoutWidget_4)
        self.rtmp_play_address.setObjectName(u"rtmp_play_address")
        self.rtmp_play_address.setReadOnly(True)

        self.rtmp_grid.addWidget(self.rtmp_play_address, 6, 1, 1, 2)

        self.label_app_name = QLabel(self.gridLayoutWidget_4)
        self.label_app_name.setObjectName(u"label_app_name")

        self.rtmp_grid.addWidget(self.label_app_name, 4, 0, 1, 1)

        self.network_ethernet_rtmp_no = QRadioButton(self.gridLayoutWidget_4)
        self.network_ethernet_rtmp_no.setObjectName(u"network_ethernet_rtmp_no")

        self.rtmp_grid.addWidget(self.network_ethernet_rtmp_no, 0, 2, 1, 1)

        self.stream_type = QComboBox(self.gridLayoutWidget_4)
        self.stream_type.addItem("")
        self.stream_type.addItem("")
        self.stream_type.setObjectName(u"stream_type")

        self.rtmp_grid.addWidget(self.stream_type, 1, 1, 1, 2)

        self.label_remote_host = QLabel(self.gridLayoutWidget_4)
        self.label_remote_host.setObjectName(u"label_remote_host")

        self.rtmp_grid.addWidget(self.label_remote_host, 2, 0, 1, 1)

        self.label_stream_id = QLabel(self.gridLayoutWidget_4)
        self.label_stream_id.setObjectName(u"label_stream_id")

        self.rtmp_grid.addWidget(self.label_stream_id, 5, 0, 1, 1)

        self.label_remote_port = QLabel(self.gridLayoutWidget_4)
        self.label_remote_port.setObjectName(u"label_remote_port")

        self.rtmp_grid.addWidget(self.label_remote_port, 3, 0, 1, 1)

        self.stream_id = QLineEdit(self.gridLayoutWidget_4)
        self.stream_id.setObjectName(u"stream_id")

        self.rtmp_grid.addWidget(self.stream_id, 5, 1, 1, 2)

        self.label_rtmp = QLabel(self.gridLayoutWidget_4)
        self.label_rtmp.setObjectName(u"label_rtmp")

        self.rtmp_grid.addWidget(self.label_rtmp, 0, 0, 1, 1)

        self.network_ethernet_rtmp_yes = QRadioButton(self.gridLayoutWidget_4)
        self.network_ethernet_rtmp_yes.setObjectName(u"network_ethernet_rtmp_yes")

        self.rtmp_grid.addWidget(self.network_ethernet_rtmp_yes, 0, 1, 1, 1)

        self.label_stream = QLabel(self.gridLayoutWidget_4)
        self.label_stream.setObjectName(u"label_stream")

        self.rtmp_grid.addWidget(self.label_stream, 1, 0, 1, 1)

        self.remote_port = QSpinBox(self.gridLayoutWidget_4)
        self.remote_port.setObjectName(u"remote_port")
        self.remote_port.setMaximum(65535)

        self.rtmp_grid.addWidget(self.remote_port, 3, 1, 1, 2)

        self.remote_host = QLineEdit(self.gridLayoutWidget_4)
        self.remote_host.setObjectName(u"remote_host")

        self.rtmp_grid.addWidget(self.remote_host, 2, 1, 1, 2)

        self.network_tab.addTab(self.network_rtmp, "")
        self.horizontalLayoutWidget = QWidget(self.network_main)
        self.horizontalLayoutWidget.setObjectName(u"horizontalLayoutWidget")
        self.horizontalLayoutWidget.setGeometry(QRect(250, 480, 160, 31))
        self.horizontalLayout = QHBoxLayout(self.horizontalLayoutWidget)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.btn_network_apply = QPushButton(self.horizontalLayoutWidget)
        self.btn_network_apply.setObjectName(u"btn_network_apply")
        self.btn_network_apply.setFlat(False)

        self.horizontalLayout.addWidget(self.btn_network_apply)

        self.btn_network_cancel = QPushButton(self.horizontalLayoutWidget)
        self.btn_network_cancel.setObjectName(u"btn_network_cancel")

        self.horizontalLayout.addWidget(self.btn_network_cancel)

        self.main_tab.addTab(self.network_main, "")
        self.media_main = QWidget()
        self.media_main.setObjectName(u"media_main")
        self.media_tab = QTabWidget(self.media_main)
        self.media_tab.setObjectName(u"media_tab")
        self.media_tab.setGeometry(QRect(10, 10, 650, 461))
        self.media_encode = QWidget()
        self.media_encode.setObjectName(u"media_encode")
        self.gridLayoutWidget_5 = QWidget(self.media_encode)
        self.gridLayoutWidget_5.setObjectName(u"gridLayoutWidget_5")
        self.gridLayoutWidget_5.setGeometry(QRect(100, 40, 401, 161))
        self.media_encode_mainstream_grid = QGridLayout(self.gridLayoutWidget_5)
        self.media_encode_mainstream_grid.setObjectName(u"media_encode_mainstream_grid")
        self.media_encode_mainstream_grid.setContentsMargins(0, 0, 0, 0)
        self.main_stream_keyframe_interval_slider = QSlider(self.gridLayoutWidget_5)
        self.main_stream_keyframe_interval_slider.setObjectName(u"main_stream_keyframe_interval_slider")
        self.main_stream_keyframe_interval_slider.setMinimum(5)
        self.main_stream_keyframe_interval_slider.setMaximum(300)
        self.main_stream_keyframe_interval_slider.setOrientation(Qt.Horizontal)

        self.media_encode_mainstream_grid.addWidget(self.main_stream_keyframe_interval_slider, 5, 2, 1, 1)

        self.label_main_key_frame = QLabel(self.gridLayoutWidget_5)
        self.label_main_key_frame.setObjectName(u"label_main_key_frame")

        self.media_encode_mainstream_grid.addWidget(self.label_main_key_frame, 5, 0, 1, 1)

        self.label_main_encode_format = QLabel(self.gridLayoutWidget_5)
        self.label_main_encode_format.setObjectName(u"label_main_encode_format")

        self.media_encode_mainstream_grid.addWidget(self.label_main_encode_format, 0, 0, 1, 1)

        self.label_main_resolution = QLabel(self.gridLayoutWidget_5)
        self.label_main_resolution.setObjectName(u"label_main_resolution")

        self.media_encode_mainstream_grid.addWidget(self.label_main_resolution, 1, 0, 1, 1)

        self.label_man_bps = QLabel(self.gridLayoutWidget_5)
        self.label_man_bps.setObjectName(u"label_man_bps")

        self.media_encode_mainstream_grid.addWidget(self.label_man_bps, 4, 0, 1, 1)

        self.label_main_framerate = QLabel(self.gridLayoutWidget_5)
        self.label_main_framerate.setObjectName(u"label_main_framerate")

        self.media_encode_mainstream_grid.addWidget(self.label_main_framerate, 3, 0, 1, 1)

        self.label_main_bitrate = QLabel(self.gridLayoutWidget_5)
        self.label_main_bitrate.setObjectName(u"label_main_bitrate")

        self.media_encode_mainstream_grid.addWidget(self.label_main_bitrate, 2, 0, 1, 1)

        self.main_stream_framerate_slider = QSlider(self.gridLayoutWidget_5)
        self.main_stream_framerate_slider.setObjectName(u"main_stream_framerate_slider")
        self.main_stream_framerate_slider.setMinimum(5)
        self.main_stream_framerate_slider.setMaximum(30)
        self.main_stream_framerate_slider.setOrientation(Qt.Horizontal)

        self.media_encode_mainstream_grid.addWidget(self.main_stream_framerate_slider, 3, 2, 1, 1)

        self.main_stream_bps_slider = QSlider(self.gridLayoutWidget_5)
        self.main_stream_bps_slider.setObjectName(u"main_stream_bps_slider")
        self.main_stream_bps_slider.setMaximumSize(QSize(8000, 16777215))
        self.main_stream_bps_slider.setMinimum(256)
        self.main_stream_bps_slider.setMaximum(8000)
        self.main_stream_bps_slider.setOrientation(Qt.Horizontal)

        self.media_encode_mainstream_grid.addWidget(self.main_stream_bps_slider, 4, 2, 1, 1)

        self.main_stream_framerate = QLineEdit(self.gridLayoutWidget_5)
        self.main_stream_framerate.setObjectName(u"main_stream_framerate")
        self.main_stream_framerate.setMaximumSize(QSize(40, 16777215))
        self.main_stream_framerate.setAlignment(Qt.AlignCenter)

        self.media_encode_mainstream_grid.addWidget(self.main_stream_framerate, 3, 1, 1, 1)

        self.main_stream_bitrate = QComboBox(self.gridLayoutWidget_5)
        self.main_stream_bitrate.addItem("")
        self.main_stream_bitrate.addItem("")
        self.main_stream_bitrate.setObjectName(u"main_stream_bitrate")

        self.media_encode_mainstream_grid.addWidget(self.main_stream_bitrate, 2, 1, 1, 2)

        self.main_stream_resolve = QComboBox(self.gridLayoutWidget_5)
        self.main_stream_resolve.setObjectName(u"main_stream_resolve")

        self.media_encode_mainstream_grid.addWidget(self.main_stream_resolve, 1, 1, 1, 2)

        self.main_stream_encode_format = QComboBox(self.gridLayoutWidget_5)
        self.main_stream_encode_format.setObjectName(u"main_stream_encode_format")

        self.media_encode_mainstream_grid.addWidget(self.main_stream_encode_format, 0, 1, 1, 2)

        self.main_stream_bps = QLineEdit(self.gridLayoutWidget_5)
        self.main_stream_bps.setObjectName(u"main_stream_bps")
        self.main_stream_bps.setMaximumSize(QSize(40, 16777215))
        self.main_stream_bps.setAlignment(Qt.AlignCenter)

        self.media_encode_mainstream_grid.addWidget(self.main_stream_bps, 4, 1, 1, 1)

        self.main_stream_keyframe_interval = QLineEdit(self.gridLayoutWidget_5)
        self.main_stream_keyframe_interval.setObjectName(u"main_stream_keyframe_interval")
        self.main_stream_keyframe_interval.setMaximumSize(QSize(40, 16777215))
        self.main_stream_keyframe_interval.setAlignment(Qt.AlignCenter)

        self.media_encode_mainstream_grid.addWidget(self.main_stream_keyframe_interval, 5, 1, 1, 1)

        self.gridLayoutWidget_6 = QWidget(self.media_encode)
        self.gridLayoutWidget_6.setObjectName(u"gridLayoutWidget_6")
        self.gridLayoutWidget_6.setGeometry(QRect(100, 240, 401, 161))
        self.media_encode_substream_grid = QGridLayout(self.gridLayoutWidget_6)
        self.media_encode_substream_grid.setObjectName(u"media_encode_substream_grid")
        self.media_encode_substream_grid.setContentsMargins(0, 0, 0, 0)
        self.label_sub_resolution = QLabel(self.gridLayoutWidget_6)
        self.label_sub_resolution.setObjectName(u"label_sub_resolution")

        self.media_encode_substream_grid.addWidget(self.label_sub_resolution, 1, 0, 1, 1)

        self.sub_stream_framerate_slider = QSlider(self.gridLayoutWidget_6)
        self.sub_stream_framerate_slider.setObjectName(u"sub_stream_framerate_slider")
        self.sub_stream_framerate_slider.setMinimum(5)
        self.sub_stream_framerate_slider.setMaximum(30)
        self.sub_stream_framerate_slider.setOrientation(Qt.Horizontal)

        self.media_encode_substream_grid.addWidget(self.sub_stream_framerate_slider, 3, 2, 1, 1)

        self.label_sub_framerate = QLabel(self.gridLayoutWidget_6)
        self.label_sub_framerate.setObjectName(u"label_sub_framerate")

        self.media_encode_substream_grid.addWidget(self.label_sub_framerate, 3, 0, 1, 1)

        self.label_sub_bps = QLabel(self.gridLayoutWidget_6)
        self.label_sub_bps.setObjectName(u"label_sub_bps")

        self.media_encode_substream_grid.addWidget(self.label_sub_bps, 4, 0, 1, 1)

        self.sub_stream_keyframe_interval_slider = QSlider(self.gridLayoutWidget_6)
        self.sub_stream_keyframe_interval_slider.setObjectName(u"sub_stream_keyframe_interval_slider")
        self.sub_stream_keyframe_interval_slider.setMinimum(5)
        self.sub_stream_keyframe_interval_slider.setMaximum(300)
        self.sub_stream_keyframe_interval_slider.setOrientation(Qt.Horizontal)

        self.media_encode_substream_grid.addWidget(self.sub_stream_keyframe_interval_slider, 5, 2, 1, 1)

        self.label_sub_encode_format = QLabel(self.gridLayoutWidget_6)
        self.label_sub_encode_format.setObjectName(u"label_sub_encode_format")

        self.media_encode_substream_grid.addWidget(self.label_sub_encode_format, 0, 0, 1, 1)

        self.sub_stream_bps_slider = QSlider(self.gridLayoutWidget_6)
        self.sub_stream_bps_slider.setObjectName(u"sub_stream_bps_slider")
        self.sub_stream_bps_slider.setMaximumSize(QSize(8000, 16777215))
        self.sub_stream_bps_slider.setMinimum(256)
        self.sub_stream_bps_slider.setMaximum(8000)
        self.sub_stream_bps_slider.setOrientation(Qt.Horizontal)

        self.media_encode_substream_grid.addWidget(self.sub_stream_bps_slider, 4, 2, 1, 1)

        self.label_sub_key_frame = QLabel(self.gridLayoutWidget_6)
        self.label_sub_key_frame.setObjectName(u"label_sub_key_frame")

        self.media_encode_substream_grid.addWidget(self.label_sub_key_frame, 5, 0, 1, 1)

        self.label_sub_bitrate = QLabel(self.gridLayoutWidget_6)
        self.label_sub_bitrate.setObjectName(u"label_sub_bitrate")

        self.media_encode_substream_grid.addWidget(self.label_sub_bitrate, 2, 0, 1, 1)

        self.sub_stream_framerate = QLineEdit(self.gridLayoutWidget_6)
        self.sub_stream_framerate.setObjectName(u"sub_stream_framerate")
        self.sub_stream_framerate.setMaximumSize(QSize(40, 16777215))
        self.sub_stream_framerate.setAlignment(Qt.AlignCenter)

        self.media_encode_substream_grid.addWidget(self.sub_stream_framerate, 3, 1, 1, 1)

        self.sub_stream_bps = QLineEdit(self.gridLayoutWidget_6)
        self.sub_stream_bps.setObjectName(u"sub_stream_bps")
        self.sub_stream_bps.setMaximumSize(QSize(40, 16777215))
        self.sub_stream_bps.setAlignment(Qt.AlignCenter)

        self.media_encode_substream_grid.addWidget(self.sub_stream_bps, 4, 1, 1, 1)

        self.sub_stream_keyframe_interval = QLineEdit(self.gridLayoutWidget_6)
        self.sub_stream_keyframe_interval.setObjectName(u"sub_stream_keyframe_interval")
        self.sub_stream_keyframe_interval.setMaximumSize(QSize(40, 16777215))
        self.sub_stream_keyframe_interval.setAlignment(Qt.AlignCenter)

        self.media_encode_substream_grid.addWidget(self.sub_stream_keyframe_interval, 5, 1, 1, 1)

        self.sub_stream_encode_format = QComboBox(self.gridLayoutWidget_6)
        self.sub_stream_encode_format.setObjectName(u"sub_stream_encode_format")

        self.media_encode_substream_grid.addWidget(self.sub_stream_encode_format, 0, 1, 1, 2)

        self.sub_stream_resolve = QComboBox(self.gridLayoutWidget_6)
        self.sub_stream_resolve.setObjectName(u"sub_stream_resolve")

        self.media_encode_substream_grid.addWidget(self.sub_stream_resolve, 1, 1, 1, 2)

        self.sub_stream_bitrate = QComboBox(self.gridLayoutWidget_6)
        self.sub_stream_bitrate.addItem("")
        self.sub_stream_bitrate.addItem("")
        self.sub_stream_bitrate.setObjectName(u"sub_stream_bitrate")

        self.media_encode_substream_grid.addWidget(self.sub_stream_bitrate, 2, 1, 1, 2)

        self.label_main_stream = QLabel(self.media_encode)
        self.label_main_stream.setObjectName(u"label_main_stream")
        self.label_main_stream.setGeometry(QRect(60, 20, 71, 16))
        font = QFont()
        font.setBold(True)
        font.setWeight(75)
        self.label_main_stream.setFont(font)
        self.label_sub_stream = QLabel(self.media_encode)
        self.label_sub_stream.setObjectName(u"label_sub_stream")
        self.label_sub_stream.setGeometry(QRect(60, 220, 71, 16))
        self.label_sub_stream.setFont(font)
        self.split_line = QFrame(self.media_encode)
        self.split_line.setObjectName(u"split_line")
        self.split_line.setGeometry(QRect(60, 210, 451, 16))
        self.split_line.setFrameShape(QFrame.HLine)
        self.split_line.setFrameShadow(QFrame.Sunken)
        self.media_tab.addTab(self.media_encode, "")
        self.media_video = QWidget()
        self.media_video.setObjectName(u"media_video")
        self.gridLayoutWidget_7 = QWidget(self.media_video)
        self.gridLayoutWidget_7.setObjectName(u"gridLayoutWidget_7")
        self.gridLayoutWidget_7.setGeometry(QRect(390, 20, 231, 201))
        self.media_video_basic_grid = QGridLayout(self.gridLayoutWidget_7)
        self.media_video_basic_grid.setObjectName(u"media_video_basic_grid")
        self.media_video_basic_grid.setContentsMargins(0, 0, 0, 0)
        self.label_saturation = QLabel(self.gridLayoutWidget_7)
        self.label_saturation.setObjectName(u"label_saturation")

        self.media_video_basic_grid.addWidget(self.label_saturation, 2, 0, 1, 1)

        self.label_brightness = QLabel(self.gridLayoutWidget_7)
        self.label_brightness.setObjectName(u"label_brightness")

        self.media_video_basic_grid.addWidget(self.label_brightness, 0, 0, 1, 1)

        self.contrast_slider = QSlider(self.gridLayoutWidget_7)
        self.contrast_slider.setObjectName(u"contrast_slider")
        self.contrast_slider.setMaximum(100)
        self.contrast_slider.setOrientation(Qt.Horizontal)

        self.media_video_basic_grid.addWidget(self.contrast_slider, 1, 2, 1, 2)

        self.vertical_flip = QCheckBox(self.gridLayoutWidget_7)
        self.vertical_flip.setObjectName(u"vertical_flip")

        self.media_video_basic_grid.addWidget(self.vertical_flip, 5, 3, 1, 1)

        self.sharpness_slider = QSlider(self.gridLayoutWidget_7)
        self.sharpness_slider.setObjectName(u"sharpness_slider")
        self.sharpness_slider.setMaximum(100)
        self.sharpness_slider.setOrientation(Qt.Horizontal)

        self.media_video_basic_grid.addWidget(self.sharpness_slider, 4, 2, 1, 2)

        self.saturation_slider = QSlider(self.gridLayoutWidget_7)
        self.saturation_slider.setObjectName(u"saturation_slider")
        self.saturation_slider.setMaximum(100)
        self.saturation_slider.setOrientation(Qt.Horizontal)

        self.media_video_basic_grid.addWidget(self.saturation_slider, 2, 2, 1, 2)

        self.backlight_slider = QSlider(self.gridLayoutWidget_7)
        self.backlight_slider.setObjectName(u"backlight_slider")
        self.backlight_slider.setMaximum(100)
        self.backlight_slider.setOrientation(Qt.Horizontal)

        self.media_video_basic_grid.addWidget(self.backlight_slider, 3, 2, 1, 2)

        self.label_contrast = QLabel(self.gridLayoutWidget_7)
        self.label_contrast.setObjectName(u"label_contrast")

        self.media_video_basic_grid.addWidget(self.label_contrast, 1, 0, 1, 1)

        self.label_sharpness = QLabel(self.gridLayoutWidget_7)
        self.label_sharpness.setObjectName(u"label_sharpness")

        self.media_video_basic_grid.addWidget(self.label_sharpness, 4, 0, 1, 1)

        self.label_backlight = QLabel(self.gridLayoutWidget_7)
        self.label_backlight.setObjectName(u"label_backlight")

        self.media_video_basic_grid.addWidget(self.label_backlight, 3, 0, 1, 1)

        self.label_flip = QLabel(self.gridLayoutWidget_7)
        self.label_flip.setObjectName(u"label_flip")

        self.media_video_basic_grid.addWidget(self.label_flip, 5, 0, 1, 1)

        self.brightness_slider = QSlider(self.gridLayoutWidget_7)
        self.brightness_slider.setObjectName(u"brightness_slider")
        self.brightness_slider.setMouseTracking(False)
        self.brightness_slider.setMaximum(100)
        self.brightness_slider.setOrientation(Qt.Horizontal)
        self.brightness_slider.setInvertedAppearance(False)
        self.brightness_slider.setTickPosition(QSlider.NoTicks)

        self.media_video_basic_grid.addWidget(self.brightness_slider, 0, 2, 1, 2)

        self.brightness = QLineEdit(self.gridLayoutWidget_7)
        self.brightness.setObjectName(u"brightness")
        self.brightness.setMaximumSize(QSize(30, 16777215))
        self.brightness.setAlignment(Qt.AlignCenter)

        self.media_video_basic_grid.addWidget(self.brightness, 0, 1, 1, 1)

        self.contrast = QLineEdit(self.gridLayoutWidget_7)
        self.contrast.setObjectName(u"contrast")
        self.contrast.setMaximumSize(QSize(30, 16777215))
        self.contrast.setAlignment(Qt.AlignCenter)

        self.media_video_basic_grid.addWidget(self.contrast, 1, 1, 1, 1)

        self.saturation = QLineEdit(self.gridLayoutWidget_7)
        self.saturation.setObjectName(u"saturation")
        self.saturation.setMaximumSize(QSize(30, 16777215))
        self.saturation.setAlignment(Qt.AlignCenter)

        self.media_video_basic_grid.addWidget(self.saturation, 2, 1, 1, 1)

        self.backlight = QLineEdit(self.gridLayoutWidget_7)
        self.backlight.setObjectName(u"backlight")
        self.backlight.setMaximumSize(QSize(30, 16777215))
        self.backlight.setAlignment(Qt.AlignCenter)

        self.media_video_basic_grid.addWidget(self.backlight, 3, 1, 1, 1)

        self.sharpness = QLineEdit(self.gridLayoutWidget_7)
        self.sharpness.setObjectName(u"sharpness")
        self.sharpness.setMaximumSize(QSize(30, 16777215))
        self.sharpness.setAlignment(Qt.AlignCenter)

        self.media_video_basic_grid.addWidget(self.sharpness, 4, 1, 1, 1)

        self.horizontal_flip = QCheckBox(self.gridLayoutWidget_7)
        self.horizontal_flip.setObjectName(u"horizontal_flip")

        self.media_video_basic_grid.addWidget(self.horizontal_flip, 5, 1, 1, 2)

        self.gridLayoutWidget_8 = QWidget(self.media_video)
        self.gridLayoutWidget_8.setObjectName(u"gridLayoutWidget_8")
        self.gridLayoutWidget_8.setGeometry(QRect(20, 280, 601, 121))
        self.media_status_advanced_grid = QGridLayout(self.gridLayoutWidget_8)
        self.media_status_advanced_grid.setObjectName(u"media_status_advanced_grid")
        self.media_status_advanced_grid.setContentsMargins(0, 0, 0, 0)
        self.AE_sensitivity = QLineEdit(self.gridLayoutWidget_8)
        self.AE_sensitivity.setObjectName(u"AE_sensitivity")
        self.AE_sensitivity.setMaximumSize(QSize(30, 16777215))
        self.AE_sensitivity.setAlignment(Qt.AlignCenter)

        self.media_status_advanced_grid.addWidget(self.AE_sensitivity, 0, 1, 1, 1)

        self.label_ae_sensitivity = QLabel(self.gridLayoutWidget_8)
        self.label_ae_sensitivity.setObjectName(u"label_ae_sensitivity")

        self.media_status_advanced_grid.addWidget(self.label_ae_sensitivity, 0, 0, 1, 1)

        self.AE_tolerance = QLineEdit(self.gridLayoutWidget_8)
        self.AE_tolerance.setObjectName(u"AE_tolerance")
        self.AE_tolerance.setMaximumSize(QSize(30, 16777215))
        self.AE_tolerance.setAlignment(Qt.AlignCenter)

        self.media_status_advanced_grid.addWidget(self.AE_tolerance, 0, 4, 1, 1)

        self.AE_tolerance_slider = QSlider(self.gridLayoutWidget_8)
        self.AE_tolerance_slider.setObjectName(u"AE_tolerance_slider")
        self.AE_tolerance_slider.setMaximum(255)
        self.AE_tolerance_slider.setOrientation(Qt.Horizontal)

        self.media_status_advanced_grid.addWidget(self.AE_tolerance_slider, 0, 5, 1, 1)

        self.label_3dnr_enhancement = QLabel(self.gridLayoutWidget_8)
        self.label_3dnr_enhancement.setObjectName(u"label_3dnr_enhancement")

        self.media_status_advanced_grid.addWidget(self.label_3dnr_enhancement, 2, 3, 1, 1)

        self.AE_sensitivity_slider = QSlider(self.gridLayoutWidget_8)
        self.AE_sensitivity_slider.setObjectName(u"AE_sensitivity_slider")
        self.AE_sensitivity_slider.setMaximum(255)
        self.AE_sensitivity_slider.setOrientation(Qt.Horizontal)

        self.media_status_advanced_grid.addWidget(self.AE_sensitivity_slider, 0, 2, 1, 1)

        self.wide_dynamic = QComboBox(self.gridLayoutWidget_8)
        self.wide_dynamic.addItem("")
        self.wide_dynamic.addItem("")
        self.wide_dynamic.addItem("")
        self.wide_dynamic.setObjectName(u"wide_dynamic")

        self.media_status_advanced_grid.addWidget(self.wide_dynamic, 1, 1, 1, 2)

        self.label_ae_tolerance = QLabel(self.gridLayoutWidget_8)
        self.label_ae_tolerance.setObjectName(u"label_ae_tolerance")

        self.media_status_advanced_grid.addWidget(self.label_ae_tolerance, 0, 3, 1, 1)

        self.noise_reduction_3d_enhancement_slider = QSlider(self.gridLayoutWidget_8)
        self.noise_reduction_3d_enhancement_slider.setObjectName(u"noise_reduction_3d_enhancement_slider")
        self.noise_reduction_3d_enhancement_slider.setMaximum(255)
        self.noise_reduction_3d_enhancement_slider.setOrientation(Qt.Horizontal)

        self.media_status_advanced_grid.addWidget(self.noise_reduction_3d_enhancement_slider, 2, 5, 1, 1)

        self.noise_reduction_3d_enhancement = QLineEdit(self.gridLayoutWidget_8)
        self.noise_reduction_3d_enhancement.setObjectName(u"noise_reduction_3d_enhancement")
        self.noise_reduction_3d_enhancement.setMaximumSize(QSize(30, 16777215))
        self.noise_reduction_3d_enhancement.setAlignment(Qt.AlignCenter)

        self.media_status_advanced_grid.addWidget(self.noise_reduction_3d_enhancement, 2, 4, 1, 1)

        self.label_wide_dynamic = QLabel(self.gridLayoutWidget_8)
        self.label_wide_dynamic.setObjectName(u"label_wide_dynamic")

        self.media_status_advanced_grid.addWidget(self.label_wide_dynamic, 1, 0, 1, 1)

        self.label_wdr_enhancement = QLabel(self.gridLayoutWidget_8)
        self.label_wdr_enhancement.setObjectName(u"label_wdr_enhancement")

        self.media_status_advanced_grid.addWidget(self.label_wdr_enhancement, 1, 3, 1, 1)

        self.wide_dynamic_enhancement = QLineEdit(self.gridLayoutWidget_8)
        self.wide_dynamic_enhancement.setObjectName(u"wide_dynamic_enhancement")
        self.wide_dynamic_enhancement.setMaximumSize(QSize(30, 16777215))
        self.wide_dynamic_enhancement.setAlignment(Qt.AlignCenter)

        self.media_status_advanced_grid.addWidget(self.wide_dynamic_enhancement, 1, 4, 1, 1)

        self.wide_dynamic_enhancement_slider = QSlider(self.gridLayoutWidget_8)
        self.wide_dynamic_enhancement_slider.setObjectName(u"wide_dynamic_enhancement_slider")
        self.wide_dynamic_enhancement_slider.setMaximum(255)
        self.wide_dynamic_enhancement_slider.setOrientation(Qt.Horizontal)

        self.media_status_advanced_grid.addWidget(self.wide_dynamic_enhancement_slider, 1, 5, 1, 1)

        self.label_3dnr = QLabel(self.gridLayoutWidget_8)
        self.label_3dnr.setObjectName(u"label_3dnr")

        self.media_status_advanced_grid.addWidget(self.label_3dnr, 2, 0, 1, 1)

        self.noise_reduction_3d = QComboBox(self.gridLayoutWidget_8)
        self.noise_reduction_3d.addItem("")
        self.noise_reduction_3d.addItem("")
        self.noise_reduction_3d.addItem("")
        self.noise_reduction_3d.setObjectName(u"noise_reduction_3d")

        self.media_status_advanced_grid.addWidget(self.noise_reduction_3d, 2, 1, 1, 2)

        self.media_video_graphic_view = QGraphicsView(self.media_video)
        self.media_video_graphic_view.setObjectName(u"media_video_graphic_view")
        self.media_video_graphic_view.setGeometry(QRect(20, 20, 350, 241))
        self.media_video_graphic_view.viewport().setProperty("cursor", QCursor(Qt.ArrowCursor))
        self.media_tab.addTab(self.media_video, "")
        self.media_audio = QWidget()
        self.media_audio.setObjectName(u"media_audio")
        self.gridLayoutWidget_9 = QWidget(self.media_audio)
        self.gridLayoutWidget_9.setObjectName(u"gridLayoutWidget_9")
        self.gridLayoutWidget_9.setGeometry(QRect(140, 30, 341, 121))
        self.media_audio_sample_grid = QGridLayout(self.gridLayoutWidget_9)
        self.media_audio_sample_grid.setObjectName(u"media_audio_sample_grid")
        self.media_audio_sample_grid.setContentsMargins(0, 0, 0, 0)
        self.label_collection_volume = QLabel(self.gridLayoutWidget_9)
        self.label_collection_volume.setObjectName(u"label_collection_volume")

        self.media_audio_sample_grid.addWidget(self.label_collection_volume, 2, 0, 1, 1)

        self.play_volume_slider = QSlider(self.gridLayoutWidget_9)
        self.play_volume_slider.setObjectName(u"play_volume_slider")
        self.play_volume_slider.setOrientation(Qt.Horizontal)

        self.media_audio_sample_grid.addWidget(self.play_volume_slider, 3, 2, 1, 1)

        self.label_samplebit = QLabel(self.gridLayoutWidget_9)
        self.label_samplebit.setObjectName(u"label_samplebit")

        self.media_audio_sample_grid.addWidget(self.label_samplebit, 1, 0, 1, 1)

        self.collect_volume_slider = QSlider(self.gridLayoutWidget_9)
        self.collect_volume_slider.setObjectName(u"collect_volume_slider")
        self.collect_volume_slider.setOrientation(Qt.Horizontal)

        self.media_audio_sample_grid.addWidget(self.collect_volume_slider, 2, 2, 1, 1)

        self.label_samplerate = QLabel(self.gridLayoutWidget_9)
        self.label_samplerate.setObjectName(u"label_samplerate")

        self.media_audio_sample_grid.addWidget(self.label_samplerate, 0, 0, 1, 1)

        self.label_play_volume = QLabel(self.gridLayoutWidget_9)
        self.label_play_volume.setObjectName(u"label_play_volume")

        self.media_audio_sample_grid.addWidget(self.label_play_volume, 3, 0, 1, 1)

        self.collect_volume = QLineEdit(self.gridLayoutWidget_9)
        self.collect_volume.setObjectName(u"collect_volume")
        self.collect_volume.setMaximumSize(QSize(30, 16777215))
        self.collect_volume.setAlignment(Qt.AlignCenter)

        self.media_audio_sample_grid.addWidget(self.collect_volume, 2, 1, 1, 1)

        self.play_volume = QLineEdit(self.gridLayoutWidget_9)
        self.play_volume.setObjectName(u"play_volume")
        self.play_volume.setMaximumSize(QSize(30, 16777215))
        self.play_volume.setAlignment(Qt.AlignCenter)

        self.media_audio_sample_grid.addWidget(self.play_volume, 3, 1, 1, 1)

        self.sample_rate = QComboBox(self.gridLayoutWidget_9)
        self.sample_rate.addItem("")
        self.sample_rate.setObjectName(u"sample_rate")

        self.media_audio_sample_grid.addWidget(self.sample_rate, 0, 1, 1, 2)

        self.sample_bit = QComboBox(self.gridLayoutWidget_9)
        self.sample_bit.addItem("")
        self.sample_bit.setObjectName(u"sample_bit")

        self.media_audio_sample_grid.addWidget(self.sample_bit, 1, 1, 1, 2)

        self.gridLayoutWidget_10 = QWidget(self.media_audio)
        self.gridLayoutWidget_10.setObjectName(u"gridLayoutWidget_10")
        self.gridLayoutWidget_10.setGeometry(QRect(140, 170, 341, 80))
        self.media_smaple_codec_grid = QGridLayout(self.gridLayoutWidget_10)
        self.media_smaple_codec_grid.setObjectName(u"media_smaple_codec_grid")
        self.media_smaple_codec_grid.setContentsMargins(0, 0, 0, 0)
        self.audio_encode_enabled_no = QRadioButton(self.gridLayoutWidget_10)
        self.audio_encode_enabled_no.setObjectName(u"audio_encode_enabled_no")

        self.media_smaple_codec_grid.addWidget(self.audio_encode_enabled_no, 0, 2, 1, 1)

        self.audio_encode_enabled_yes = QRadioButton(self.gridLayoutWidget_10)
        self.audio_encode_enabled_yes.setObjectName(u"audio_encode_enabled_yes")

        self.media_smaple_codec_grid.addWidget(self.audio_encode_enabled_yes, 0, 1, 1, 1)

        self.label_audio_codec = QLabel(self.gridLayoutWidget_10)
        self.label_audio_codec.setObjectName(u"label_audio_codec")

        self.media_smaple_codec_grid.addWidget(self.label_audio_codec, 0, 0, 1, 1)

        self.label_audio_format = QLabel(self.gridLayoutWidget_10)
        self.label_audio_format.setObjectName(u"label_audio_format")

        self.media_smaple_codec_grid.addWidget(self.label_audio_format, 1, 0, 1, 1)

        self.media_audio_codec_format = QComboBox(self.gridLayoutWidget_10)
        self.media_audio_codec_format.addItem("")
        self.media_audio_codec_format.addItem("")
        self.media_audio_codec_format.setObjectName(u"media_audio_codec_format")

        self.media_smaple_codec_grid.addWidget(self.media_audio_codec_format, 1, 1, 1, 2)

        self.media_tab.addTab(self.media_audio, "")
        self.media_osd = QWidget()
        self.media_osd.setObjectName(u"media_osd")
        self.gridLayoutWidget_11 = QWidget(self.media_osd)
        self.gridLayoutWidget_11.setObjectName(u"gridLayoutWidget_11")
        self.gridLayoutWidget_11.setGeometry(QRect(40, 300, 291, 97))
        self.media_osd_title_grid = QGridLayout(self.gridLayoutWidget_11)
        self.media_osd_title_grid.setObjectName(u"media_osd_title_grid")
        self.media_osd_title_grid.setContentsMargins(0, 0, 0, 0)
        self.osd_title_name = QLineEdit(self.gridLayoutWidget_11)
        self.osd_title_name.setObjectName(u"osd_title_name")

        self.media_osd_title_grid.addWidget(self.osd_title_name, 3, 1, 1, 2)

        self.label_osd_title = QLabel(self.gridLayoutWidget_11)
        self.label_osd_title.setObjectName(u"label_osd_title")

        self.media_osd_title_grid.addWidget(self.label_osd_title, 3, 0, 1, 1)

        self.osd_title_y = QDoubleSpinBox(self.gridLayoutWidget_11)
        self.osd_title_y.setObjectName(u"osd_title_y")

        self.media_osd_title_grid.addWidget(self.osd_title_y, 2, 1, 1, 2)

        self.label_osd_title_y = QLabel(self.gridLayoutWidget_11)
        self.label_osd_title_y.setObjectName(u"label_osd_title_y")

        self.media_osd_title_grid.addWidget(self.label_osd_title_y, 2, 0, 1, 1)

        self.osd_title_x = QDoubleSpinBox(self.gridLayoutWidget_11)
        self.osd_title_x.setObjectName(u"osd_title_x")

        self.media_osd_title_grid.addWidget(self.osd_title_x, 1, 1, 1, 2)

        self.label_osd_title_x = QLabel(self.gridLayoutWidget_11)
        self.label_osd_title_x.setObjectName(u"label_osd_title_x")

        self.media_osd_title_grid.addWidget(self.label_osd_title_x, 1, 0, 1, 1)

        self.osd_title_no = QRadioButton(self.gridLayoutWidget_11)
        self.media_osd_title_btngrp = QButtonGroup(MainWindow)
        self.media_osd_title_btngrp.setObjectName(u"media_osd_title_btngrp")
        self.media_osd_title_btngrp.addButton(self.osd_title_no)
        self.osd_title_no.setObjectName(u"osd_title_no")

        self.media_osd_title_grid.addWidget(self.osd_title_no, 0, 2, 1, 1)

        self.osd_title_yes = QRadioButton(self.gridLayoutWidget_11)
        self.media_osd_title_btngrp.addButton(self.osd_title_yes)
        self.osd_title_yes.setObjectName(u"osd_title_yes")

        self.media_osd_title_grid.addWidget(self.osd_title_yes, 0, 1, 1, 1)

        self.label_osd_enable_title = QLabel(self.gridLayoutWidget_11)
        self.label_osd_enable_title.setObjectName(u"label_osd_enable_title")

        self.media_osd_title_grid.addWidget(self.label_osd_enable_title, 0, 0, 1, 1)

        self.gridLayoutWidget_12 = QWidget(self.media_osd)
        self.gridLayoutWidget_12.setObjectName(u"gridLayoutWidget_12")
        self.gridLayoutWidget_12.setGeometry(QRect(360, 300, 241, 121))
        self.media_osd_datetime_grid = QGridLayout(self.gridLayoutWidget_12)
        self.media_osd_datetime_grid.setObjectName(u"media_osd_datetime_grid")
        self.media_osd_datetime_grid.setContentsMargins(0, 0, 0, 0)
        self.osd_datetime_yes = QRadioButton(self.gridLayoutWidget_12)
        self.osd_datetime_yes.setObjectName(u"osd_datetime_yes")

        self.media_osd_datetime_grid.addWidget(self.osd_datetime_yes, 0, 1, 1, 1)

        self.label_osd_time_format = QLabel(self.gridLayoutWidget_12)
        self.label_osd_time_format.setObjectName(u"label_osd_time_format")

        self.media_osd_datetime_grid.addWidget(self.label_osd_time_format, 3, 0, 1, 1)

        self.osd_datetime_time_format = QComboBox(self.gridLayoutWidget_12)
        self.osd_datetime_time_format.addItem("")
        self.osd_datetime_time_format.addItem("")
        self.osd_datetime_time_format.setObjectName(u"osd_datetime_time_format")

        self.media_osd_datetime_grid.addWidget(self.osd_datetime_time_format, 3, 1, 1, 2)

        self.label_osd_datetime_y = QLabel(self.gridLayoutWidget_12)
        self.label_osd_datetime_y.setObjectName(u"label_osd_datetime_y")

        self.media_osd_datetime_grid.addWidget(self.label_osd_datetime_y, 2, 0, 1, 1)

        self.label_osd_date_format = QLabel(self.gridLayoutWidget_12)
        self.label_osd_date_format.setObjectName(u"label_osd_date_format")

        self.media_osd_datetime_grid.addWidget(self.label_osd_date_format, 4, 0, 1, 1)

        self.label_osd_datetime_x = QLabel(self.gridLayoutWidget_12)
        self.label_osd_datetime_x.setObjectName(u"label_osd_datetime_x")

        self.media_osd_datetime_grid.addWidget(self.label_osd_datetime_x, 1, 0, 1, 1)

        self.osd_datetime_no = QRadioButton(self.gridLayoutWidget_12)
        self.osd_datetime_no.setObjectName(u"osd_datetime_no")

        self.media_osd_datetime_grid.addWidget(self.osd_datetime_no, 0, 2, 1, 1)

        self.osd_datetime_date_format = QComboBox(self.gridLayoutWidget_12)
        self.osd_datetime_date_format.addItem("")
        self.osd_datetime_date_format.addItem("")
        self.osd_datetime_date_format.addItem("")
        self.osd_datetime_date_format.addItem("")
        self.osd_datetime_date_format.addItem("")
        self.osd_datetime_date_format.addItem("")
        self.osd_datetime_date_format.setObjectName(u"osd_datetime_date_format")

        self.media_osd_datetime_grid.addWidget(self.osd_datetime_date_format, 4, 1, 1, 2)

        self.osd_datetime_x = QDoubleSpinBox(self.gridLayoutWidget_12)
        self.osd_datetime_x.setObjectName(u"osd_datetime_x")

        self.media_osd_datetime_grid.addWidget(self.osd_datetime_x, 1, 1, 1, 2)

        self.osd_datetime_y = QDoubleSpinBox(self.gridLayoutWidget_12)
        self.osd_datetime_y.setObjectName(u"osd_datetime_y")

        self.media_osd_datetime_grid.addWidget(self.osd_datetime_y, 2, 1, 1, 2)

        self.label_osd_enable_datetime = QLabel(self.gridLayoutWidget_12)
        self.label_osd_enable_datetime.setObjectName(u"label_osd_enable_datetime")

        self.media_osd_datetime_grid.addWidget(self.label_osd_enable_datetime, 0, 0, 1, 1)

        self.media_osd_graphic_view = QGraphicsView(self.media_osd)
        self.media_osd_graphic_view.setObjectName(u"media_osd_graphic_view")
        self.media_osd_graphic_view.setGeometry(QRect(63, 10, 510, 281))
        self.media_osd_graphic_view.viewport().setProperty("cursor", QCursor(Qt.ArrowCursor))
        self.media_tab.addTab(self.media_osd, "")
        self.media_privacy = QWidget()
        self.media_privacy.setObjectName(u"media_privacy")
        self.gridLayoutWidget_13 = QWidget(self.media_privacy)
        self.gridLayoutWidget_13.setObjectName(u"gridLayoutWidget_13")
        self.gridLayoutWidget_13.setGeometry(QRect(210, 310, 221, 119))
        self.media_privacy_grid = QGridLayout(self.gridLayoutWidget_13)
        self.media_privacy_grid.setObjectName(u"media_privacy_grid")
        self.media_privacy_grid.setContentsMargins(0, 0, 0, 0)
        self.privacy_zone3_enabled = QCheckBox(self.gridLayoutWidget_13)
        self.privacy_zone3_enabled.setObjectName(u"privacy_zone3_enabled")

        self.media_privacy_grid.addWidget(self.privacy_zone3_enabled, 4, 1, 1, 1, Qt.AlignHCenter)

        self.label_privacymask_enable = QLabel(self.gridLayoutWidget_13)
        self.label_privacymask_enable.setObjectName(u"label_privacymask_enable")

        self.media_privacy_grid.addWidget(self.label_privacymask_enable, 0, 1, 1, 1, Qt.AlignHCenter)

        self.privacy_zone1_sel = QRadioButton(self.gridLayoutWidget_13)
        self.media_privacy_zone_btngrp = QButtonGroup(MainWindow)
        self.media_privacy_zone_btngrp.setObjectName(u"media_privacy_zone_btngrp")
        self.media_privacy_zone_btngrp.addButton(self.privacy_zone1_sel)
        self.privacy_zone1_sel.setObjectName(u"privacy_zone1_sel")

        self.media_privacy_grid.addWidget(self.privacy_zone1_sel, 2, 0, 1, 1, Qt.AlignHCenter)

        self.privacy_zone0_enabled = QCheckBox(self.gridLayoutWidget_13)
        self.privacy_zone0_enabled.setObjectName(u"privacy_zone0_enabled")
        self.privacy_zone0_enabled.setTristate(False)

        self.media_privacy_grid.addWidget(self.privacy_zone0_enabled, 1, 1, 1, 1, Qt.AlignHCenter)

        self.privacy_zone2_enabled = QCheckBox(self.gridLayoutWidget_13)
        self.privacy_zone2_enabled.setObjectName(u"privacy_zone2_enabled")

        self.media_privacy_grid.addWidget(self.privacy_zone2_enabled, 3, 1, 1, 1, Qt.AlignHCenter)

        self.privacy_zone2_sel = QRadioButton(self.gridLayoutWidget_13)
        self.media_privacy_zone_btngrp.addButton(self.privacy_zone2_sel)
        self.privacy_zone2_sel.setObjectName(u"privacy_zone2_sel")

        self.media_privacy_grid.addWidget(self.privacy_zone2_sel, 3, 0, 1, 1, Qt.AlignHCenter)

        self.privacy_zone3_sel = QRadioButton(self.gridLayoutWidget_13)
        self.media_privacy_zone_btngrp.addButton(self.privacy_zone3_sel)
        self.privacy_zone3_sel.setObjectName(u"privacy_zone3_sel")

        self.media_privacy_grid.addWidget(self.privacy_zone3_sel, 4, 0, 1, 1, Qt.AlignHCenter)

        self.privacy_zone0_sel = QRadioButton(self.gridLayoutWidget_13)
        self.media_privacy_zone_btngrp.addButton(self.privacy_zone0_sel)
        self.privacy_zone0_sel.setObjectName(u"privacy_zone0_sel")
        self.privacy_zone0_sel.setLayoutDirection(Qt.LeftToRight)
        self.privacy_zone0_sel.setChecked(True)

        self.media_privacy_grid.addWidget(self.privacy_zone0_sel, 1, 0, 1, 1, Qt.AlignHCenter)

        self.privacy_zone1_enabled = QCheckBox(self.gridLayoutWidget_13)
        self.privacy_zone1_enabled.setObjectName(u"privacy_zone1_enabled")

        self.media_privacy_grid.addWidget(self.privacy_zone1_enabled, 2, 1, 1, 1, Qt.AlignHCenter)

        self.label_privacymask_zone = QLabel(self.gridLayoutWidget_13)
        self.label_privacymask_zone.setObjectName(u"label_privacymask_zone")

        self.media_privacy_grid.addWidget(self.label_privacymask_zone, 0, 0, 1, 1, Qt.AlignHCenter)

        self.media_privacy_graphic_view = QGraphicsView(self.media_privacy)
        self.media_privacy_graphic_view.setObjectName(u"media_privacy_graphic_view")
        self.media_privacy_graphic_view.setGeometry(QRect(63, 10, 510, 281))
        self.media_privacy_graphic_view.viewport().setProperty("cursor", QCursor(Qt.CrossCursor))
        self.media_tab.addTab(self.media_privacy, "")
        self.horizontalLayoutWidget_2 = QWidget(self.media_main)
        self.horizontalLayoutWidget_2.setObjectName(u"horizontalLayoutWidget_2")
        self.horizontalLayoutWidget_2.setGeometry(QRect(250, 480, 160, 31))
        self.horizontalLayout_2 = QHBoxLayout(self.horizontalLayoutWidget_2)
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.btn_media_apply = QPushButton(self.horizontalLayoutWidget_2)
        self.btn_media_apply.setObjectName(u"btn_media_apply")

        self.horizontalLayout_2.addWidget(self.btn_media_apply)

        self.btn_media_cancel = QPushButton(self.horizontalLayoutWidget_2)
        self.btn_media_cancel.setObjectName(u"btn_media_cancel")

        self.horizontalLayout_2.addWidget(self.btn_media_cancel)

        self.main_tab.addTab(self.media_main, "")
        self.detection_main = QWidget()
        self.detection_main.setObjectName(u"detection_main")
        self.detection_tab = QTabWidget(self.detection_main)
        self.detection_tab.setObjectName(u"detection_tab")
        self.detection_tab.setGeometry(QRect(10, 10, 650, 461))
        self.detect_human = QWidget()
        self.detect_human.setObjectName(u"detect_human")
        self.gridLayoutWidget_14 = QWidget(self.detect_human)
        self.gridLayoutWidget_14.setObjectName(u"gridLayoutWidget_14")
        self.gridLayoutWidget_14.setGeometry(QRect(110, 370, 431, 51))
        self.detect_human_grid = QGridLayout(self.gridLayoutWidget_14)
        self.detect_human_grid.setObjectName(u"detect_human_grid")
        self.detect_human_grid.setContentsMargins(0, 0, 0, 0)
        self.label_detection_human_enable = QLabel(self.gridLayoutWidget_14)
        self.label_detection_human_enable.setObjectName(u"label_detection_human_enable")

        self.detect_human_grid.addWidget(self.label_detection_human_enable, 0, 0, 1, 1)

        self.label_detection_human_sensitivity = QLabel(self.gridLayoutWidget_14)
        self.label_detection_human_sensitivity.setObjectName(u"label_detection_human_sensitivity")

        self.detect_human_grid.addWidget(self.label_detection_human_sensitivity, 1, 0, 1, 1)

        self.radio_detect_human_zone2 = QRadioButton(self.gridLayoutWidget_14)
        self.detect_human_zone_btn_grp = QButtonGroup(MainWindow)
        self.detect_human_zone_btn_grp.setObjectName(u"detect_human_zone_btn_grp")
        self.detect_human_zone_btn_grp.addButton(self.radio_detect_human_zone2)
        self.radio_detect_human_zone2.setObjectName(u"radio_detect_human_zone2")
        self.radio_detect_human_zone2.setLayoutDirection(Qt.RightToLeft)

        self.detect_human_grid.addWidget(self.radio_detect_human_zone2, 0, 6, 1, 1)

        self.detection_human_enable_no = QRadioButton(self.gridLayoutWidget_14)
        self.detect_human_btngrp = QButtonGroup(MainWindow)
        self.detect_human_btngrp.setObjectName(u"detect_human_btngrp")
        self.detect_human_btngrp.addButton(self.detection_human_enable_no)
        self.detection_human_enable_no.setObjectName(u"detection_human_enable_no")

        self.detect_human_grid.addWidget(self.detection_human_enable_no, 0, 2, 1, 1)

        self.radio_detect_human_zone1 = QRadioButton(self.gridLayoutWidget_14)
        self.detect_human_zone_btn_grp.addButton(self.radio_detect_human_zone1)
        self.radio_detect_human_zone1.setObjectName(u"radio_detect_human_zone1")
        self.radio_detect_human_zone1.setLayoutDirection(Qt.RightToLeft)

        self.detect_human_grid.addWidget(self.radio_detect_human_zone1, 0, 5, 1, 1)

        self.radio_detect_human_zone0 = QRadioButton(self.gridLayoutWidget_14)
        self.detect_human_zone_btn_grp.addButton(self.radio_detect_human_zone0)
        self.radio_detect_human_zone0.setObjectName(u"radio_detect_human_zone0")
        self.radio_detect_human_zone0.setLayoutDirection(Qt.RightToLeft)
        self.radio_detect_human_zone0.setChecked(True)

        self.detect_human_grid.addWidget(self.radio_detect_human_zone0, 0, 4, 1, 1)

        self.label_detection_human_zone = QLabel(self.gridLayoutWidget_14)
        self.label_detection_human_zone.setObjectName(u"label_detection_human_zone")
        self.label_detection_human_zone.setLayoutDirection(Qt.LeftToRight)
        self.label_detection_human_zone.setAlignment(Qt.AlignRight|Qt.AlignTrailing|Qt.AlignVCenter)

        self.detect_human_grid.addWidget(self.label_detection_human_zone, 0, 3, 1, 1)

        self.detection_human_enable_yes = QRadioButton(self.gridLayoutWidget_14)
        self.detect_human_btngrp.addButton(self.detection_human_enable_yes)
        self.detection_human_enable_yes.setObjectName(u"detection_human_enable_yes")

        self.detect_human_grid.addWidget(self.detection_human_enable_yes, 0, 1, 1, 1)

        self.detection_human_sensitivity = QLineEdit(self.gridLayoutWidget_14)
        self.detection_human_sensitivity.setObjectName(u"detection_human_sensitivity")
        self.detection_human_sensitivity.setMaximumSize(QSize(30, 16777215))
        self.detection_human_sensitivity.setAlignment(Qt.AlignCenter)

        self.detect_human_grid.addWidget(self.detection_human_sensitivity, 1, 1, 1, 1)

        self.radio_detect_human_zone3 = QRadioButton(self.gridLayoutWidget_14)
        self.detect_human_zone_btn_grp.addButton(self.radio_detect_human_zone3)
        self.radio_detect_human_zone3.setObjectName(u"radio_detect_human_zone3")
        self.radio_detect_human_zone3.setLayoutDirection(Qt.RightToLeft)

        self.detect_human_grid.addWidget(self.radio_detect_human_zone3, 0, 7, 1, 1)

        self.detection_human_sensitivity_slider = QSlider(self.gridLayoutWidget_14)
        self.detection_human_sensitivity_slider.setObjectName(u"detection_human_sensitivity_slider")
        self.detection_human_sensitivity_slider.setOrientation(Qt.Horizontal)

        self.detect_human_grid.addWidget(self.detection_human_sensitivity_slider, 1, 2, 1, 6)

        self.detection_human_graphic_view = QGraphicsView(self.detect_human)
        self.detection_human_graphic_view.setObjectName(u"detection_human_graphic_view")
        self.detection_human_graphic_view.setGeometry(QRect(50, 10, 542, 342))
        self.detection_human_graphic_view.viewport().setProperty("cursor", QCursor(Qt.CrossCursor))
        self.detection_tab.addTab(self.detect_human, "")
        self.detect_motion = QWidget()
        self.detect_motion.setObjectName(u"detect_motion")
        self.gridLayoutWidget_15 = QWidget(self.detect_motion)
        self.gridLayoutWidget_15.setObjectName(u"gridLayoutWidget_15")
        self.gridLayoutWidget_15.setGeometry(QRect(110, 370, 431, 51))
        self.detect_motion_grid = QGridLayout(self.gridLayoutWidget_15)
        self.detect_motion_grid.setObjectName(u"detect_motion_grid")
        self.detect_motion_grid.setContentsMargins(0, 0, 0, 0)
        self.label_detect_motion_enable = QLabel(self.gridLayoutWidget_15)
        self.label_detect_motion_enable.setObjectName(u"label_detect_motion_enable")

        self.detect_motion_grid.addWidget(self.label_detect_motion_enable, 0, 0, 1, 1)

        self.detection_motion_enable_no = QRadioButton(self.gridLayoutWidget_15)
        self.detec_motion_btngrp = QButtonGroup(MainWindow)
        self.detec_motion_btngrp.setObjectName(u"detec_motion_btngrp")
        self.detec_motion_btngrp.addButton(self.detection_motion_enable_no)
        self.detection_motion_enable_no.setObjectName(u"detection_motion_enable_no")

        self.detect_motion_grid.addWidget(self.detection_motion_enable_no, 0, 2, 1, 1)

        self.detection_motion_enable_yes = QRadioButton(self.gridLayoutWidget_15)
        self.detec_motion_btngrp.addButton(self.detection_motion_enable_yes)
        self.detection_motion_enable_yes.setObjectName(u"detection_motion_enable_yes")

        self.detect_motion_grid.addWidget(self.detection_motion_enable_yes, 0, 1, 1, 1)

        self.label_detect_motion_sensitivity = QLabel(self.gridLayoutWidget_15)
        self.label_detect_motion_sensitivity.setObjectName(u"label_detect_motion_sensitivity")

        self.detect_motion_grid.addWidget(self.label_detect_motion_sensitivity, 1, 0, 1, 1)

        self.detection_motion_sensitivity_slider = QSlider(self.gridLayoutWidget_15)
        self.detection_motion_sensitivity_slider.setObjectName(u"detection_motion_sensitivity_slider")
        self.detection_motion_sensitivity_slider.setOrientation(Qt.Horizontal)

        self.detect_motion_grid.addWidget(self.detection_motion_sensitivity_slider, 1, 2, 1, 1)

        self.detection_motion_sensitivity = QLineEdit(self.gridLayoutWidget_15)
        self.detection_motion_sensitivity.setObjectName(u"detection_motion_sensitivity")
        self.detection_motion_sensitivity.setMaximumSize(QSize(30, 16777215))
        self.detection_motion_sensitivity.setAlignment(Qt.AlignCenter)

        self.detect_motion_grid.addWidget(self.detection_motion_sensitivity, 1, 1, 1, 1)

        self.detection_motion_graphic_view = QGraphicsView(self.detect_motion)
        self.detection_motion_graphic_view.setObjectName(u"detection_motion_graphic_view")
        self.detection_motion_graphic_view.setGeometry(QRect(50, 10, 542, 342))
        self.detection_motion_graphic_view.viewport().setProperty("cursor", QCursor(Qt.CrossCursor))
        self.detection_tab.addTab(self.detect_motion, "")
        self.detect_tamper = QWidget()
        self.detect_tamper.setObjectName(u"detect_tamper")
        self.gridLayoutWidget_16 = QWidget(self.detect_tamper)
        self.gridLayoutWidget_16.setObjectName(u"gridLayoutWidget_16")
        self.gridLayoutWidget_16.setGeometry(QRect(110, 370, 431, 51))
        self.detect_tamper_grid = QGridLayout(self.gridLayoutWidget_16)
        self.detect_tamper_grid.setObjectName(u"detect_tamper_grid")
        self.detect_tamper_grid.setContentsMargins(0, 0, 0, 0)
        self.label_detection_tamper_enable = QLabel(self.gridLayoutWidget_16)
        self.label_detection_tamper_enable.setObjectName(u"label_detection_tamper_enable")

        self.detect_tamper_grid.addWidget(self.label_detection_tamper_enable, 0, 0, 1, 1)

        self.detection_tamper_enable_no = QRadioButton(self.gridLayoutWidget_16)
        self.detec_tamper_btngrp = QButtonGroup(MainWindow)
        self.detec_tamper_btngrp.setObjectName(u"detec_tamper_btngrp")
        self.detec_tamper_btngrp.addButton(self.detection_tamper_enable_no)
        self.detection_tamper_enable_no.setObjectName(u"detection_tamper_enable_no")

        self.detect_tamper_grid.addWidget(self.detection_tamper_enable_no, 0, 2, 1, 1)

        self.detection_tamper_enable_yes = QRadioButton(self.gridLayoutWidget_16)
        self.detec_tamper_btngrp.addButton(self.detection_tamper_enable_yes)
        self.detection_tamper_enable_yes.setObjectName(u"detection_tamper_enable_yes")

        self.detect_tamper_grid.addWidget(self.detection_tamper_enable_yes, 0, 1, 1, 1)

        self.label_detection_tamper_sensitivity = QLabel(self.gridLayoutWidget_16)
        self.label_detection_tamper_sensitivity.setObjectName(u"label_detection_tamper_sensitivity")

        self.detect_tamper_grid.addWidget(self.label_detection_tamper_sensitivity, 1, 0, 1, 1)

        self.detection_tamper_sensitivity_slider = QSlider(self.gridLayoutWidget_16)
        self.detection_tamper_sensitivity_slider.setObjectName(u"detection_tamper_sensitivity_slider")
        self.detection_tamper_sensitivity_slider.setMaximum(100)
        self.detection_tamper_sensitivity_slider.setSingleStep(25)
        self.detection_tamper_sensitivity_slider.setPageStep(25)
        self.detection_tamper_sensitivity_slider.setOrientation(Qt.Horizontal)

        self.detect_tamper_grid.addWidget(self.detection_tamper_sensitivity_slider, 1, 2, 1, 1)

        self.detection_tamper_sensitivity = QLineEdit(self.gridLayoutWidget_16)
        self.detection_tamper_sensitivity.setObjectName(u"detection_tamper_sensitivity")
        self.detection_tamper_sensitivity.setMaximumSize(QSize(30, 16777215))
        self.detection_tamper_sensitivity.setAlignment(Qt.AlignCenter)

        self.detect_tamper_grid.addWidget(self.detection_tamper_sensitivity, 1, 1, 1, 1)

        self.detection_tamper_graphic_view = QGraphicsView(self.detect_tamper)
        self.detection_tamper_graphic_view.setObjectName(u"detection_tamper_graphic_view")
        self.detection_tamper_graphic_view.setGeometry(QRect(50, 10, 542, 342))
        self.detection_tamper_graphic_view.viewport().setProperty("cursor", QCursor(Qt.ArrowCursor))
        self.detection_tab.addTab(self.detect_tamper, "")
        self.horizontalLayoutWidget_3 = QWidget(self.detection_main)
        self.horizontalLayoutWidget_3.setObjectName(u"horizontalLayoutWidget_3")
        self.horizontalLayoutWidget_3.setGeometry(QRect(250, 480, 160, 31))
        self.horizontalLayout_3 = QHBoxLayout(self.horizontalLayoutWidget_3)
        self.horizontalLayout_3.setObjectName(u"horizontalLayout_3")
        self.horizontalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.btn_detection_apply = QPushButton(self.horizontalLayoutWidget_3)
        self.btn_detection_apply.setObjectName(u"btn_detection_apply")

        self.horizontalLayout_3.addWidget(self.btn_detection_apply)

        self.btn_detection_cancel = QPushButton(self.horizontalLayoutWidget_3)
        self.btn_detection_cancel.setObjectName(u"btn_detection_cancel")

        self.horizontalLayout_3.addWidget(self.btn_detection_cancel)

        self.main_tab.addTab(self.detection_main, "")
        self.peripherial_main = QWidget()
        self.peripherial_main.setObjectName(u"peripherial_main")
        self.peripherial_tab = QTabWidget(self.peripherial_main)
        self.peripherial_tab.setObjectName(u"peripherial_tab")
        self.peripherial_tab.setGeometry(QRect(10, 10, 650, 461))
        self.peri_alarm = QWidget()
        self.peri_alarm.setObjectName(u"peri_alarm")
        self.peripherial_tab.addTab(self.peri_alarm, "")
        self.peri_serial = QWidget()
        self.peri_serial.setObjectName(u"peri_serial")
        self.peripherial_tab.addTab(self.peri_serial, "")
        self.peri_ptz = QWidget()
        self.peri_ptz.setObjectName(u"peri_ptz")
        self.peripherial_tab.addTab(self.peri_ptz, "")
        self.peri_sd = QWidget()
        self.peri_sd.setObjectName(u"peri_sd")
        self.peripherial_tab.addTab(self.peri_sd, "")
        self.horizontalLayoutWidget_4 = QWidget(self.peripherial_main)
        self.horizontalLayoutWidget_4.setObjectName(u"horizontalLayoutWidget_4")
        self.horizontalLayoutWidget_4.setGeometry(QRect(250, 480, 160, 31))
        self.horizontalLayout_4 = QHBoxLayout(self.horizontalLayoutWidget_4)
        self.horizontalLayout_4.setObjectName(u"horizontalLayout_4")
        self.horizontalLayout_4.setContentsMargins(0, 0, 0, 0)
        self.btn_peripherial_apply = QPushButton(self.horizontalLayoutWidget_4)
        self.btn_peripherial_apply.setObjectName(u"btn_peripherial_apply")

        self.horizontalLayout_4.addWidget(self.btn_peripherial_apply)

        self.btn_peripherial_cancel = QPushButton(self.horizontalLayoutWidget_4)
        self.btn_peripherial_cancel.setObjectName(u"btn_peripherial_cancel")

        self.horizontalLayout_4.addWidget(self.btn_peripherial_cancel)

        self.main_tab.addTab(self.peripherial_main, "")
        self.system_main = QWidget()
        self.system_main.setObjectName(u"system_main")
        self.system_tab = QTabWidget(self.system_main)
        self.system_tab.setObjectName(u"system_tab")
        self.system_tab.setGeometry(QRect(10, 10, 650, 491))
        self.system_user = QWidget()
        self.system_user.setObjectName(u"system_user")
        self.user_tree = QTreeWidget(self.system_user)
        self.user_tree.setObjectName(u"user_tree")
        self.user_tree.setGeometry(QRect(90, 20, 441, 192))
        self.horizontalLayoutWidget_6 = QWidget(self.system_user)
        self.horizontalLayoutWidget_6.setObjectName(u"horizontalLayoutWidget_6")
        self.horizontalLayoutWidget_6.setGeometry(QRect(220, 420, 239, 31))
        self.horizontalLayout_6 = QHBoxLayout(self.horizontalLayoutWidget_6)
        self.horizontalLayout_6.setObjectName(u"horizontalLayout_6")
        self.horizontalLayout_6.setContentsMargins(0, 0, 0, 0)
        self.btn_system_user_add = QPushButton(self.horizontalLayoutWidget_6)
        self.btn_system_user_add.setObjectName(u"btn_system_user_add")

        self.horizontalLayout_6.addWidget(self.btn_system_user_add)

        self.btn_system_user_modify = QPushButton(self.horizontalLayoutWidget_6)
        self.btn_system_user_modify.setObjectName(u"btn_system_user_modify")

        self.horizontalLayout_6.addWidget(self.btn_system_user_modify)

        self.btn_system_user_delete = QPushButton(self.horizontalLayoutWidget_6)
        self.btn_system_user_delete.setObjectName(u"btn_system_user_delete")

        self.horizontalLayout_6.addWidget(self.btn_system_user_delete)

        self.system_user_info_widget = QWidget(self.system_user)
        self.system_user_info_widget.setObjectName(u"system_user_info_widget")
        self.system_user_info_widget.setGeometry(QRect(90, 230, 441, 151))
        self.gridLayoutWidget_20 = QWidget(self.system_user_info_widget)
        self.gridLayoutWidget_20.setObjectName(u"gridLayoutWidget_20")
        self.gridLayoutWidget_20.setGeometry(QRect(50, 0, 341, 141))
        self.grid_system_user_info = QGridLayout(self.gridLayoutWidget_20)
        self.grid_system_user_info.setObjectName(u"grid_system_user_info")
        self.grid_system_user_info.setContentsMargins(0, 0, 0, 0)
        self.label_password2 = QLabel(self.gridLayoutWidget_20)
        self.label_password2.setObjectName(u"label_password2")

        self.grid_system_user_info.addWidget(self.label_password2, 2, 0, 1, 1)

        self.system_user_yes = QRadioButton(self.gridLayoutWidget_20)
        self.system_user_enabled_btngrp = QButtonGroup(MainWindow)
        self.system_user_enabled_btngrp.setObjectName(u"system_user_enabled_btngrp")
        self.system_user_enabled_btngrp.addButton(self.system_user_yes)
        self.system_user_yes.setObjectName(u"system_user_yes")

        self.grid_system_user_info.addWidget(self.system_user_yes, 4, 1, 1, 1)

        self.label_user_group = QLabel(self.gridLayoutWidget_20)
        self.label_user_group.setObjectName(u"label_user_group")

        self.grid_system_user_info.addWidget(self.label_user_group, 3, 0, 1, 1)

        self.label_password = QLabel(self.gridLayoutWidget_20)
        self.label_password.setObjectName(u"label_password")

        self.grid_system_user_info.addWidget(self.label_password, 1, 0, 1, 1)

        self.label_user_enabled = QLabel(self.gridLayoutWidget_20)
        self.label_user_enabled.setObjectName(u"label_user_enabled")

        self.grid_system_user_info.addWidget(self.label_user_enabled, 4, 0, 1, 1)

        self.system_user_no = QRadioButton(self.gridLayoutWidget_20)
        self.system_user_enabled_btngrp.addButton(self.system_user_no)
        self.system_user_no.setObjectName(u"system_user_no")

        self.grid_system_user_info.addWidget(self.system_user_no, 4, 2, 1, 1)

        self.system_user_name = QLineEdit(self.gridLayoutWidget_20)
        self.system_user_name.setObjectName(u"system_user_name")

        self.grid_system_user_info.addWidget(self.system_user_name, 0, 1, 1, 2)

        self.system_user_group = QComboBox(self.gridLayoutWidget_20)
        self.system_user_group.addItem("")
        self.system_user_group.addItem("")
        self.system_user_group.addItem("")
        self.system_user_group.setObjectName(u"system_user_group")

        self.grid_system_user_info.addWidget(self.system_user_group, 3, 1, 1, 2)

        self.system_user_password2 = QLineEdit(self.gridLayoutWidget_20)
        self.system_user_password2.setObjectName(u"system_user_password2")
        self.system_user_password2.setEchoMode(QLineEdit.Password)

        self.grid_system_user_info.addWidget(self.system_user_password2, 2, 1, 1, 2)

        self.system_user_password = QLineEdit(self.gridLayoutWidget_20)
        self.system_user_password.setObjectName(u"system_user_password")
        self.system_user_password.setEchoMode(QLineEdit.Password)

        self.grid_system_user_info.addWidget(self.system_user_password, 1, 1, 1, 2)

        self.label_username = QLabel(self.gridLayoutWidget_20)
        self.label_username.setObjectName(u"label_username")

        self.grid_system_user_info.addWidget(self.label_username, 0, 0, 1, 1)

        self.system_user_name_old = QLineEdit(self.system_user)
        self.system_user_name_old.setObjectName(u"system_user_name_old")
        self.system_user_name_old.setGeometry(QRect(493, 234, 31, 20))
        self.system_tab.addTab(self.system_user, "")
        self.system_datetime = QWidget()
        self.system_datetime.setObjectName(u"system_datetime")
        self.label_camera_current_datetime = QLabel(self.system_datetime)
        self.label_camera_current_datetime.setObjectName(u"label_camera_current_datetime")
        self.label_camera_current_datetime.setGeometry(QRect(120, 20, 211, 16))
        self.label_camera_current_datetime.setFont(font)
        self.device_datetime = QLabel(self.system_datetime)
        self.device_datetime.setObjectName(u"device_datetime")
        self.device_datetime.setGeometry(QRect(140, 40, 121, 16))
        self.frame = QFrame(self.system_datetime)
        self.frame.setObjectName(u"frame")
        self.frame.setGeometry(QRect(110, 60, 411, 161))
        self.frame.setFrameShape(QFrame.StyledPanel)
        self.frame.setFrameShadow(QFrame.Raised)
        self.label_camera_datetime_set = QLabel(self.frame)
        self.label_camera_datetime_set.setObjectName(u"label_camera_datetime_set")
        self.label_camera_datetime_set.setGeometry(QRect(10, 10, 244, 15))
        self.label_camera_datetime_set.setFont(font)
        self.set_datetime_with_pc = QRadioButton(self.frame)
        self.sysem_datetime_set_btngrp = QButtonGroup(MainWindow)
        self.sysem_datetime_set_btngrp.setObjectName(u"sysem_datetime_set_btngrp")
        self.sysem_datetime_set_btngrp.addButton(self.set_datetime_with_pc)
        self.set_datetime_with_pc.setObjectName(u"set_datetime_with_pc")
        self.set_datetime_with_pc.setGeometry(QRect(30, 30, 259, 17))
        self.pc_datetime = QLabel(self.frame)
        self.pc_datetime.setObjectName(u"pc_datetime")
        self.pc_datetime.setGeometry(QRect(50, 50, 259, 21))
        self.pc_datetime.setMargin(0)
        self.pc_datetime.setIndent(-1)
        self.set_datetime_manual = QRadioButton(self.frame)
        self.sysem_datetime_set_btngrp.addButton(self.set_datetime_manual)
        self.set_datetime_manual.setObjectName(u"set_datetime_manual")
        self.set_datetime_manual.setGeometry(QRect(30, 70, 259, 17))
        self.system_datetime_date = QDateEdit(self.frame)
        self.system_datetime_date.setObjectName(u"system_datetime_date")
        self.system_datetime_date.setGeometry(QRect(50, 90, 127, 20))
        self.system_datetime_time = QTimeEdit(self.frame)
        self.system_datetime_time.setObjectName(u"system_datetime_time")
        self.system_datetime_time.setGeometry(QRect(200, 90, 101, 20))
        self.system_datetime_time.setTime(QTime(0, 0, 7))
        self.horizontalLayoutWidget_13 = QWidget(self.frame)
        self.horizontalLayoutWidget_13.setObjectName(u"horizontalLayoutWidget_13")
        self.horizontalLayoutWidget_13.setGeometry(QRect(120, 120, 179, 31))
        self.horizontalLayout_13 = QHBoxLayout(self.horizontalLayoutWidget_13)
        self.horizontalLayout_13.setObjectName(u"horizontalLayout_13")
        self.horizontalLayout_13.setContentsMargins(0, 0, 0, 0)
        self.btn_system_datetime_set = QPushButton(self.horizontalLayoutWidget_13)
        self.btn_system_datetime_set.setObjectName(u"btn_system_datetime_set")

        self.horizontalLayout_13.addWidget(self.btn_system_datetime_set)

        self.frame_2 = QFrame(self.system_datetime)
        self.frame_2.setObjectName(u"frame_2")
        self.frame_2.setGeometry(QRect(110, 240, 411, 211))
        self.frame_2.setFrameShape(QFrame.StyledPanel)
        self.frame_2.setFrameShadow(QFrame.Raised)
        self.label_datetime_configuration = QLabel(self.frame_2)
        self.label_datetime_configuration.setObjectName(u"label_datetime_configuration")
        self.label_datetime_configuration.setGeometry(QRect(10, 0, 171, 16))
        self.label_datetime_configuration.setFont(font)
        self.gridLayoutWidget_2 = QWidget(self.frame_2)
        self.gridLayoutWidget_2.setObjectName(u"gridLayoutWidget_2")
        self.gridLayoutWidget_2.setGeometry(QRect(20, 20, 371, 131))
        self.system_datetime_grid = QGridLayout(self.gridLayoutWidget_2)
        self.system_datetime_grid.setObjectName(u"system_datetime_grid")
        self.system_datetime_grid.setContentsMargins(0, 0, 0, 0)
        self.label_ntp_server = QLabel(self.gridLayoutWidget_2)
        self.label_ntp_server.setObjectName(u"label_ntp_server")

        self.system_datetime_grid.addWidget(self.label_ntp_server, 2, 0, 1, 1)

        self.label_timezone = QLabel(self.gridLayoutWidget_2)
        self.label_timezone.setObjectName(u"label_timezone")

        self.system_datetime_grid.addWidget(self.label_timezone, 1, 0, 1, 1)

        self.radio_ntp_yes = QRadioButton(self.gridLayoutWidget_2)
        self.radio_ntp_yes.setObjectName(u"radio_ntp_yes")

        self.system_datetime_grid.addWidget(self.radio_ntp_yes, 0, 1, 1, 1)

        self.label_enable_ntp = QLabel(self.gridLayoutWidget_2)
        self.label_enable_ntp.setObjectName(u"label_enable_ntp")

        self.system_datetime_grid.addWidget(self.label_enable_ntp, 0, 0, 1, 1)

        self.radio_ntp_no = QRadioButton(self.gridLayoutWidget_2)
        self.radio_ntp_no.setObjectName(u"radio_ntp_no")

        self.system_datetime_grid.addWidget(self.radio_ntp_no, 0, 2, 1, 1)

        self.sync_time = QSpinBox(self.gridLayoutWidget_2)
        self.sync_time.setObjectName(u"sync_time")

        self.system_datetime_grid.addWidget(self.sync_time, 4, 1, 1, 2)

        self.label_ntp_sync_time = QLabel(self.gridLayoutWidget_2)
        self.label_ntp_sync_time.setObjectName(u"label_ntp_sync_time")

        self.system_datetime_grid.addWidget(self.label_ntp_sync_time, 4, 0, 1, 1)

        self.timezone = QComboBox(self.gridLayoutWidget_2)
        self.timezone.setObjectName(u"timezone")

        self.system_datetime_grid.addWidget(self.timezone, 1, 1, 1, 2)

        self.ntp_server = QLineEdit(self.gridLayoutWidget_2)
        self.ntp_server.setObjectName(u"ntp_server")

        self.system_datetime_grid.addWidget(self.ntp_server, 2, 1, 1, 2)

        self.label_ntp_port = QLabel(self.gridLayoutWidget_2)
        self.label_ntp_port.setObjectName(u"label_ntp_port")

        self.system_datetime_grid.addWidget(self.label_ntp_port, 3, 0, 1, 1)

        self.ntp_port = QSpinBox(self.gridLayoutWidget_2)
        self.ntp_port.setObjectName(u"ntp_port")
        self.ntp_port.setMaximum(65535)

        self.system_datetime_grid.addWidget(self.ntp_port, 3, 1, 1, 2)

        self.horizontalLayoutWidget_5 = QWidget(self.frame_2)
        self.horizontalLayoutWidget_5.setObjectName(u"horizontalLayoutWidget_5")
        self.horizontalLayoutWidget_5.setGeometry(QRect(130, 170, 160, 31))
        self.horizontalLayout_5 = QHBoxLayout(self.horizontalLayoutWidget_5)
        self.horizontalLayout_5.setObjectName(u"horizontalLayout_5")
        self.horizontalLayout_5.setContentsMargins(0, 0, 0, 0)
        self.btn_system_dateteime_apply = QPushButton(self.horizontalLayoutWidget_5)
        self.btn_system_dateteime_apply.setObjectName(u"btn_system_dateteime_apply")

        self.horizontalLayout_5.addWidget(self.btn_system_dateteime_apply)

        self.btn_system_datetime_cancel = QPushButton(self.horizontalLayoutWidget_5)
        self.btn_system_datetime_cancel.setObjectName(u"btn_system_datetime_cancel")

        self.horizontalLayout_5.addWidget(self.btn_system_datetime_cancel)

        self.system_tab.addTab(self.system_datetime, "")
        self.system_maintenance = QWidget()
        self.system_maintenance.setObjectName(u"system_maintenance")
        self.gridLayoutWidget_18 = QWidget(self.system_maintenance)
        self.gridLayoutWidget_18.setObjectName(u"gridLayoutWidget_18")
        self.gridLayoutWidget_18.setGeometry(QRect(110, 40, 331, 141))
        self.system_maintenance_restart_grid = QGridLayout(self.gridLayoutWidget_18)
        self.system_maintenance_restart_grid.setObjectName(u"system_maintenance_restart_grid")
        self.system_maintenance_restart_grid.setContentsMargins(0, 0, 0, 0)
        self.btn_system_restart = QPushButton(self.gridLayoutWidget_18)
        self.btn_system_restart.setObjectName(u"btn_system_restart")

        self.system_maintenance_restart_grid.addWidget(self.btn_system_restart, 0, 1, 1, 1)

        self.label_restart = QLabel(self.gridLayoutWidget_18)
        self.label_restart.setObjectName(u"label_restart")
        self.label_restart.setFont(font)

        self.system_maintenance_restart_grid.addWidget(self.label_restart, 0, 0, 1, 1)

        self.label_restore_factory_default = QLabel(self.gridLayoutWidget_18)
        self.label_restore_factory_default.setObjectName(u"label_restore_factory_default")
        self.label_restore_factory_default.setFont(font)

        self.system_maintenance_restart_grid.addWidget(self.label_restore_factory_default, 1, 0, 1, 1)

        self.btn_system_restore = QPushButton(self.gridLayoutWidget_18)
        self.btn_system_restore.setObjectName(u"btn_system_restore")

        self.system_maintenance_restart_grid.addWidget(self.btn_system_restore, 1, 1, 1, 1)

        self.gridLayoutWidget_17 = QWidget(self.system_maintenance)
        self.gridLayoutWidget_17.setObjectName(u"gridLayoutWidget_17")
        self.gridLayoutWidget_17.setGeometry(QRect(110, 270, 412, 101))
        self.system_maintenance_schedule_grid = QGridLayout(self.gridLayoutWidget_17)
        self.system_maintenance_schedule_grid.setObjectName(u"system_maintenance_schedule_grid")
        self.system_maintenance_schedule_grid.setContentsMargins(0, 0, 0, 0)
        self.label_maintenance_enable = QLabel(self.gridLayoutWidget_17)
        self.label_maintenance_enable.setObjectName(u"label_maintenance_enable")

        self.system_maintenance_schedule_grid.addWidget(self.label_maintenance_enable, 2, 0, 1, 1)

        self.cksm_fri = QCheckBox(self.gridLayoutWidget_17)
        self.cksm_fri.setObjectName(u"cksm_fri")

        self.system_maintenance_schedule_grid.addWidget(self.cksm_fri, 2, 6, 1, 1)

        self.system_restart_time = QTimeEdit(self.gridLayoutWidget_17)
        self.system_restart_time.setObjectName(u"system_restart_time")

        self.system_maintenance_schedule_grid.addWidget(self.system_restart_time, 3, 1, 1, 3)

        self.cksm_sun = QCheckBox(self.gridLayoutWidget_17)
        self.cksm_sun.setObjectName(u"cksm_sun")

        self.system_maintenance_schedule_grid.addWidget(self.cksm_sun, 2, 1, 1, 1)

        self.system_maintenance_no = QRadioButton(self.gridLayoutWidget_17)
        self.system_maintenance_no.setObjectName(u"system_maintenance_no")

        self.system_maintenance_schedule_grid.addWidget(self.system_maintenance_no, 1, 3, 1, 2)

        self.system_maintenance_yes = QRadioButton(self.gridLayoutWidget_17)
        self.system_maintenance_yes.setObjectName(u"system_maintenance_yes")

        self.system_maintenance_schedule_grid.addWidget(self.system_maintenance_yes, 1, 1, 1, 2)

        self.cksm_wed = QCheckBox(self.gridLayoutWidget_17)
        self.cksm_wed.setObjectName(u"cksm_wed")

        self.system_maintenance_schedule_grid.addWidget(self.cksm_wed, 2, 4, 1, 1)

        self.cksm_thu = QCheckBox(self.gridLayoutWidget_17)
        self.cksm_thu.setObjectName(u"cksm_thu")

        self.system_maintenance_schedule_grid.addWidget(self.cksm_thu, 2, 5, 1, 1)

        self.cksm_mon = QCheckBox(self.gridLayoutWidget_17)
        self.cksm_mon.setObjectName(u"cksm_mon")

        self.system_maintenance_schedule_grid.addWidget(self.cksm_mon, 2, 2, 1, 1)

        self.cksm_tue = QCheckBox(self.gridLayoutWidget_17)
        self.cksm_tue.setObjectName(u"cksm_tue")

        self.system_maintenance_schedule_grid.addWidget(self.cksm_tue, 2, 3, 1, 1)

        self.cksm_sat = QCheckBox(self.gridLayoutWidget_17)
        self.cksm_sat.setObjectName(u"cksm_sat")

        self.system_maintenance_schedule_grid.addWidget(self.cksm_sat, 2, 7, 1, 1)

        self.label_maintenace_restart_time = QLabel(self.gridLayoutWidget_17)
        self.label_maintenace_restart_time.setObjectName(u"label_maintenace_restart_time")

        self.system_maintenance_schedule_grid.addWidget(self.label_maintenace_restart_time, 3, 0, 1, 1)

        self.label_system_maintenance_enable = QLabel(self.gridLayoutWidget_17)
        self.label_system_maintenance_enable.setObjectName(u"label_system_maintenance_enable")

        self.system_maintenance_schedule_grid.addWidget(self.label_system_maintenance_enable, 1, 0, 1, 1)

        self.label_system_maintenance_schedule = QLabel(self.gridLayoutWidget_17)
        self.label_system_maintenance_schedule.setObjectName(u"label_system_maintenance_schedule")
        self.label_system_maintenance_schedule.setFont(font)

        self.system_maintenance_schedule_grid.addWidget(self.label_system_maintenance_schedule, 0, 0, 1, 8)

        self.horizontalLayoutWidget_7 = QWidget(self.system_maintenance)
        self.horizontalLayoutWidget_7.setObjectName(u"horizontalLayoutWidget_7")
        self.horizontalLayoutWidget_7.setGeometry(QRect(230, 400, 160, 31))
        self.horizontalLayout_7 = QHBoxLayout(self.horizontalLayoutWidget_7)
        self.horizontalLayout_7.setObjectName(u"horizontalLayout_7")
        self.horizontalLayout_7.setContentsMargins(0, 0, 0, 0)
        self.btn_system_maintenance_apply = QPushButton(self.horizontalLayoutWidget_7)
        self.btn_system_maintenance_apply.setObjectName(u"btn_system_maintenance_apply")

        self.horizontalLayout_7.addWidget(self.btn_system_maintenance_apply)

        self.btn_system_maintenance_cancel = QPushButton(self.horizontalLayoutWidget_7)
        self.btn_system_maintenance_cancel.setObjectName(u"btn_system_maintenance_cancel")

        self.horizontalLayout_7.addWidget(self.btn_system_maintenance_cancel)

        self.system_tab.addTab(self.system_maintenance, "")
        self.system_about = QWidget()
        self.system_about.setObjectName(u"system_about")
        self.gridLayoutWidget_19 = QWidget(self.system_about)
        self.gridLayoutWidget_19.setObjectName(u"gridLayoutWidget_19")
        self.gridLayoutWidget_19.setGeometry(QRect(120, 40, 351, 181))
        self.system_about_grid = QGridLayout(self.gridLayoutWidget_19)
        self.system_about_grid.setObjectName(u"system_about_grid")
        self.system_about_grid.setContentsMargins(0, 0, 0, 0)
        self.label_device_version = QLabel(self.gridLayoutWidget_19)
        self.label_device_version.setObjectName(u"label_device_version")

        self.system_about_grid.addWidget(self.label_device_version, 2, 0, 1, 1)

        self.label_device_name = QLabel(self.gridLayoutWidget_19)
        self.label_device_name.setObjectName(u"label_device_name")

        self.system_about_grid.addWidget(self.label_device_name, 0, 0, 1, 1)

        self.label_device_uptime = QLabel(self.gridLayoutWidget_19)
        self.label_device_uptime.setObjectName(u"label_device_uptime")

        self.system_about_grid.addWidget(self.label_device_uptime, 1, 0, 1, 1)

        self.label_device_firmware = QLabel(self.gridLayoutWidget_19)
        self.label_device_firmware.setObjectName(u"label_device_firmware")

        self.system_about_grid.addWidget(self.label_device_firmware, 3, 0, 1, 1)

        self.system_about_device_name = QLineEdit(self.gridLayoutWidget_19)
        self.system_about_device_name.setObjectName(u"system_about_device_name")
        self.system_about_device_name.setReadOnly(True)

        self.system_about_grid.addWidget(self.system_about_device_name, 0, 1, 1, 1)

        self.system_about_device_uptime = QLineEdit(self.gridLayoutWidget_19)
        self.system_about_device_uptime.setObjectName(u"system_about_device_uptime")
        self.system_about_device_uptime.setReadOnly(True)

        self.system_about_grid.addWidget(self.system_about_device_uptime, 1, 1, 1, 1)

        self.system_about_device_version = QLineEdit(self.gridLayoutWidget_19)
        self.system_about_device_version.setObjectName(u"system_about_device_version")
        self.system_about_device_version.setReadOnly(True)

        self.system_about_grid.addWidget(self.system_about_device_version, 2, 1, 1, 1)

        self.system_about_firmware_version = QLineEdit(self.gridLayoutWidget_19)
        self.system_about_firmware_version.setObjectName(u"system_about_firmware_version")
        self.system_about_firmware_version.setReadOnly(True)

        self.system_about_grid.addWidget(self.system_about_firmware_version, 3, 1, 1, 1)

        self.system_tab.addTab(self.system_about, "")
        self.main_tab.addTab(self.system_main, "")
        MainWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(MainWindow)

        self.main_tab.setCurrentIndex(0)
        self.network_tab.setCurrentIndex(0)
        self.btn_network_apply.setDefault(False)
        self.media_tab.setCurrentIndex(0)
        self.detection_tab.setCurrentIndex(0)
        self.peripherial_tab.setCurrentIndex(0)
        self.system_tab.setCurrentIndex(0)


        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"장치관리자", None))
        self.label_dhcp.setText(QCoreApplication.translate("MainWindow", u"DHCP", None))
        self.label_gateway.setText(QCoreApplication.translate("MainWindow", u"Gateway", None))
        self.label_mac.setText(QCoreApplication.translate("MainWindow", u"Mac Address", None))
        self.label_ip_address.setText(QCoreApplication.translate("MainWindow", u"IP Address", None))
        self.label_subnetmask.setText(QCoreApplication.translate("MainWindow", u"Subnet mask", None))
        self.label_dns2.setText(QCoreApplication.translate("MainWindow", u"DNS2", None))
        self.label_ip_self_adaption.setText(QCoreApplication.translate("MainWindow", u"IP Seff adaption", None))
        self.label_dns1.setText(QCoreApplication.translate("MainWindow", u"DNS1", None))
        self.radio_ip_self_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.radio_ip_self_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.network_ethernet_dhcp_yes.setText(QCoreApplication.translate("MainWindow", u"예", None))
        self.network_ethernet_dhcp_no.setText(QCoreApplication.translate("MainWindow", u"아니오", None))
        self.network_tab.setTabText(self.network_tab.indexOf(self.network_ethernet), QCoreApplication.translate("MainWindow", u"Ethernet", None))
        self.label_multicast_play_address.setText(QCoreApplication.translate("MainWindow", u"UDP Multicast Play Address", None))
        self.label_muticast_ip.setText(QCoreApplication.translate("MainWindow", u"Multicast IP", None))
        self.label_multicast_port.setText(QCoreApplication.translate("MainWindow", u"Multicast Port", None))
        self.network_ethernet_udp_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_videostream.setText(QCoreApplication.translate("MainWindow", u"Video Stream", None))
        self.label_open_multitask.setText(QCoreApplication.translate("MainWindow", u"Open Multicast", None))
        self.network_ethernet_udp_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.network_udp_stream.setItemText(0, QCoreApplication.translate("MainWindow", u"Main stream", None))
        self.network_udp_stream.setItemText(1, QCoreApplication.translate("MainWindow", u"Sub stream", None))

        self.network_tab.setTabText(self.network_tab.indexOf(self.network_utp), QCoreApplication.translate("MainWindow", u"UDP", None))
        self.label_rtmp_play_address.setText(QCoreApplication.translate("MainWindow", u"RTMP Play Address", None))
        self.label_app_name.setText(QCoreApplication.translate("MainWindow", u"App Name", None))
        self.network_ethernet_rtmp_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.stream_type.setItemText(0, QCoreApplication.translate("MainWindow", u"Main stream", None))
        self.stream_type.setItemText(1, QCoreApplication.translate("MainWindow", u"Sub stream", None))

        self.label_remote_host.setText(QCoreApplication.translate("MainWindow", u"Remote Host", None))
        self.label_stream_id.setText(QCoreApplication.translate("MainWindow", u"Stream ID", None))
        self.label_remote_port.setText(QCoreApplication.translate("MainWindow", u"Remote Port", None))
        self.label_rtmp.setText(QCoreApplication.translate("MainWindow", u"RTMP", None))
        self.network_ethernet_rtmp_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_stream.setText(QCoreApplication.translate("MainWindow", u"Stream", None))
        self.network_tab.setTabText(self.network_tab.indexOf(self.network_rtmp), QCoreApplication.translate("MainWindow", u"RTMP", None))
        self.btn_network_apply.setText(QCoreApplication.translate("MainWindow", u"Apply", None))
        self.btn_network_cancel.setText(QCoreApplication.translate("MainWindow", u"Cancel", None))
        self.main_tab.setTabText(self.main_tab.indexOf(self.network_main), QCoreApplication.translate("MainWindow", u"Network", None))
        self.label_main_key_frame.setText(QCoreApplication.translate("MainWindow", u"Key frame interval", None))
        self.label_main_encode_format.setText(QCoreApplication.translate("MainWindow", u"Encode Format", None))
        self.label_main_resolution.setText(QCoreApplication.translate("MainWindow", u"Resolution", None))
        self.label_man_bps.setText(QCoreApplication.translate("MainWindow", u"Bitrate", None))
        self.label_main_framerate.setText(QCoreApplication.translate("MainWindow", u"Framerate", None))
        self.label_main_bitrate.setText(QCoreApplication.translate("MainWindow", u"Bitrate", None))
        self.main_stream_bitrate.setItemText(0, QCoreApplication.translate("MainWindow", u"CBR", None))
        self.main_stream_bitrate.setItemText(1, QCoreApplication.translate("MainWindow", u"VBR", None))

        self.main_stream_bps.setText("")
        self.label_sub_resolution.setText(QCoreApplication.translate("MainWindow", u"Resolution", None))
        self.label_sub_framerate.setText(QCoreApplication.translate("MainWindow", u"Framerate", None))
        self.label_sub_bps.setText(QCoreApplication.translate("MainWindow", u"Bitrate", None))
        self.label_sub_encode_format.setText(QCoreApplication.translate("MainWindow", u"Encode Format", None))
        self.label_sub_key_frame.setText(QCoreApplication.translate("MainWindow", u"Key frame interval", None))
        self.label_sub_bitrate.setText(QCoreApplication.translate("MainWindow", u"Bitrate", None))
        self.sub_stream_bitrate.setItemText(0, QCoreApplication.translate("MainWindow", u"CBR", None))
        self.sub_stream_bitrate.setItemText(1, QCoreApplication.translate("MainWindow", u"VBR", None))

        self.label_main_stream.setText(QCoreApplication.translate("MainWindow", u"Main Stream", None))
        self.label_sub_stream.setText(QCoreApplication.translate("MainWindow", u"Sub Stream", None))
        self.media_tab.setTabText(self.media_tab.indexOf(self.media_encode), QCoreApplication.translate("MainWindow", u"Encode", None))
        self.label_saturation.setText(QCoreApplication.translate("MainWindow", u"Saturation", None))
        self.label_brightness.setText(QCoreApplication.translate("MainWindow", u"Brightness", None))
        self.vertical_flip.setText(QCoreApplication.translate("MainWindow", u"Vertical", None))
        self.label_contrast.setText(QCoreApplication.translate("MainWindow", u"Contrast", None))
        self.label_sharpness.setText(QCoreApplication.translate("MainWindow", u"Sharpness", None))
        self.label_backlight.setText(QCoreApplication.translate("MainWindow", u"Backlight", None))
        self.label_flip.setText(QCoreApplication.translate("MainWindow", u"Flip", None))
#if QT_CONFIG(statustip)
        self.brightness_slider.setStatusTip("")
#endif // QT_CONFIG(statustip)
        self.horizontal_flip.setText(QCoreApplication.translate("MainWindow", u"Horizontal", None))
        self.label_ae_sensitivity.setText(QCoreApplication.translate("MainWindow", u"AE Sensitivity", None))
        self.label_3dnr_enhancement.setText(QCoreApplication.translate("MainWindow", u"3D NR Enhancement", None))
        self.wide_dynamic.setItemText(0, QCoreApplication.translate("MainWindow", u"Disable", None))
        self.wide_dynamic.setItemText(1, QCoreApplication.translate("MainWindow", u"Auto", None))
        self.wide_dynamic.setItemText(2, QCoreApplication.translate("MainWindow", u"Manual", None))

        self.label_ae_tolerance.setText(QCoreApplication.translate("MainWindow", u"AE Tolerance", None))
        self.label_wide_dynamic.setText(QCoreApplication.translate("MainWindow", u"Wide Dynamic", None))
        self.label_wdr_enhancement.setText(QCoreApplication.translate("MainWindow", u"WDR Enhancement", None))
        self.label_3dnr.setText(QCoreApplication.translate("MainWindow", u"3D Noisce Reduction", None))
        self.noise_reduction_3d.setItemText(0, QCoreApplication.translate("MainWindow", u"Disable", None))
        self.noise_reduction_3d.setItemText(1, QCoreApplication.translate("MainWindow", u"Auto", None))
        self.noise_reduction_3d.setItemText(2, QCoreApplication.translate("MainWindow", u"Manual", None))

        self.media_tab.setTabText(self.media_tab.indexOf(self.media_video), QCoreApplication.translate("MainWindow", u"Video", None))
        self.label_collection_volume.setText(QCoreApplication.translate("MainWindow", u"Collect Volumn", None))
        self.label_samplebit.setText(QCoreApplication.translate("MainWindow", u"Sample Bit", None))
        self.label_samplerate.setText(QCoreApplication.translate("MainWindow", u"Sample Rate", None))
        self.label_play_volume.setText(QCoreApplication.translate("MainWindow", u"Play Volumn", None))
        self.sample_rate.setItemText(0, QCoreApplication.translate("MainWindow", u"8000", None))

        self.sample_bit.setItemText(0, QCoreApplication.translate("MainWindow", u"16", None))

        self.audio_encode_enabled_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.audio_encode_enabled_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_audio_codec.setText(QCoreApplication.translate("MainWindow", u"Codec", None))
        self.label_audio_format.setText(QCoreApplication.translate("MainWindow", u"Format", None))
        self.media_audio_codec_format.setItemText(0, QCoreApplication.translate("MainWindow", u"G711A", None))
        self.media_audio_codec_format.setItemText(1, QCoreApplication.translate("MainWindow", u"G711U", None))

        self.media_tab.setTabText(self.media_tab.indexOf(self.media_audio), QCoreApplication.translate("MainWindow", u"Audio", None))
        self.label_osd_title.setText(QCoreApplication.translate("MainWindow", u"Title", None))
        self.label_osd_title_y.setText(QCoreApplication.translate("MainWindow", u"Y Coordinate(%)", None))
        self.label_osd_title_x.setText(QCoreApplication.translate("MainWindow", u"X coordinate(%)", None))
        self.osd_title_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.osd_title_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_osd_enable_title.setText(QCoreApplication.translate("MainWindow", u"Enable Title", None))
        self.osd_datetime_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_osd_time_format.setText(QCoreApplication.translate("MainWindow", u"Time Format", None))
        self.osd_datetime_time_format.setItemText(0, QCoreApplication.translate("MainWindow", u"24 Hours", None))
        self.osd_datetime_time_format.setItemText(1, QCoreApplication.translate("MainWindow", u"12 Hours", None))

        self.label_osd_datetime_y.setText(QCoreApplication.translate("MainWindow", u"Y Coordinate(%)", None))
        self.label_osd_date_format.setText(QCoreApplication.translate("MainWindow", u"Date Format", None))
        self.label_osd_datetime_x.setText(QCoreApplication.translate("MainWindow", u"X Coordinate(%)", None))
        self.osd_datetime_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.osd_datetime_date_format.setItemText(0, QCoreApplication.translate("MainWindow", u"YYYY-MM-DD(2020-10-15)", None))
        self.osd_datetime_date_format.setItemText(1, QCoreApplication.translate("MainWindow", u"MM-DD-YYYY(10-15-2020)", None))
        self.osd_datetime_date_format.setItemText(2, QCoreApplication.translate("MainWindow", u"DD-MM-YYYY(15-10-2020)", None))
        self.osd_datetime_date_format.setItemText(3, QCoreApplication.translate("MainWindow", u"YYYY/MM/DD(2020/10/15)", None))
        self.osd_datetime_date_format.setItemText(4, QCoreApplication.translate("MainWindow", u"MM/DD/YYYY(10/15/2020)", None))
        self.osd_datetime_date_format.setItemText(5, QCoreApplication.translate("MainWindow", u"DD/MM/YYYY(15/10/2020)", None))

        self.label_osd_enable_datetime.setText(QCoreApplication.translate("MainWindow", u"Date & Time", None))
        self.media_tab.setTabText(self.media_tab.indexOf(self.media_osd), QCoreApplication.translate("MainWindow", u"OSD", None))
        self.privacy_zone3_enabled.setText("")
        self.label_privacymask_enable.setText(QCoreApplication.translate("MainWindow", u"Enable", None))
        self.privacy_zone1_sel.setText(QCoreApplication.translate("MainWindow", u"2", None))
        self.privacy_zone0_enabled.setText("")
        self.privacy_zone2_enabled.setText("")
        self.privacy_zone2_sel.setText(QCoreApplication.translate("MainWindow", u"3", None))
        self.privacy_zone3_sel.setText(QCoreApplication.translate("MainWindow", u"4", None))
        self.privacy_zone0_sel.setText(QCoreApplication.translate("MainWindow", u"1", None))
        self.privacy_zone1_enabled.setText("")
        self.label_privacymask_zone.setText(QCoreApplication.translate("MainWindow", u"Zone", None))
        self.media_tab.setTabText(self.media_tab.indexOf(self.media_privacy), QCoreApplication.translate("MainWindow", u"Privacy Mask", None))
        self.btn_media_apply.setText(QCoreApplication.translate("MainWindow", u"Apply", None))
        self.btn_media_cancel.setText(QCoreApplication.translate("MainWindow", u"Cancel", None))
        self.main_tab.setTabText(self.main_tab.indexOf(self.media_main), QCoreApplication.translate("MainWindow", u"Media", None))
        self.label_detection_human_enable.setText(QCoreApplication.translate("MainWindow", u"Enable", None))
        self.label_detection_human_sensitivity.setText(QCoreApplication.translate("MainWindow", u"Sensitvity", None))
        self.radio_detect_human_zone2.setText(QCoreApplication.translate("MainWindow", u"3", None))
        self.detection_human_enable_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.radio_detect_human_zone1.setText(QCoreApplication.translate("MainWindow", u"2", None))
        self.radio_detect_human_zone0.setText(QCoreApplication.translate("MainWindow", u"1", None))
        self.label_detection_human_zone.setText(QCoreApplication.translate("MainWindow", u"Zone", None))
        self.detection_human_enable_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.radio_detect_human_zone3.setText(QCoreApplication.translate("MainWindow", u"4", None))
        self.detection_tab.setTabText(self.detection_tab.indexOf(self.detect_human), QCoreApplication.translate("MainWindow", u"Human", None))
        self.label_detect_motion_enable.setText(QCoreApplication.translate("MainWindow", u"Enable", None))
        self.detection_motion_enable_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.detection_motion_enable_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_detect_motion_sensitivity.setText(QCoreApplication.translate("MainWindow", u"Sensitvity", None))
        self.detection_tab.setTabText(self.detection_tab.indexOf(self.detect_motion), QCoreApplication.translate("MainWindow", u"Motion", None))
        self.label_detection_tamper_enable.setText(QCoreApplication.translate("MainWindow", u"Enable", None))
        self.detection_tamper_enable_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.detection_tamper_enable_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_detection_tamper_sensitivity.setText(QCoreApplication.translate("MainWindow", u"Sensitvity", None))
        self.detection_tab.setTabText(self.detection_tab.indexOf(self.detect_tamper), QCoreApplication.translate("MainWindow", u"Tamper", None))
        self.btn_detection_apply.setText(QCoreApplication.translate("MainWindow", u"Apply", None))
        self.btn_detection_cancel.setText(QCoreApplication.translate("MainWindow", u"Cancel", None))
        self.main_tab.setTabText(self.main_tab.indexOf(self.detection_main), QCoreApplication.translate("MainWindow", u"Detection", None))
        self.peripherial_tab.setTabText(self.peripherial_tab.indexOf(self.peri_alarm), QCoreApplication.translate("MainWindow", u"Alarm", None))
        self.peripherial_tab.setTabText(self.peripherial_tab.indexOf(self.peri_serial), QCoreApplication.translate("MainWindow", u"Serial", None))
        self.peripherial_tab.setTabText(self.peripherial_tab.indexOf(self.peri_ptz), QCoreApplication.translate("MainWindow", u"PTZ", None))
        self.peripherial_tab.setTabText(self.peripherial_tab.indexOf(self.peri_sd), QCoreApplication.translate("MainWindow", u"SD", None))
        self.btn_peripherial_apply.setText(QCoreApplication.translate("MainWindow", u"Apply", None))
        self.btn_peripherial_cancel.setText(QCoreApplication.translate("MainWindow", u"Cancel", None))
        self.main_tab.setTabText(self.main_tab.indexOf(self.peripherial_main), QCoreApplication.translate("MainWindow", u"Peripherial", None))
        ___qtreewidgetitem = self.user_tree.headerItem()
        ___qtreewidgetitem.setText(3, QCoreApplication.translate("MainWindow", u"Enable", None));
        ___qtreewidgetitem.setText(2, QCoreApplication.translate("MainWindow", u"Group", None));
        ___qtreewidgetitem.setText(1, QCoreApplication.translate("MainWindow", u"Name", None));
        ___qtreewidgetitem.setText(0, QCoreApplication.translate("MainWindow", u"No", None));
        self.btn_system_user_add.setText(QCoreApplication.translate("MainWindow", u"Add", None))
        self.btn_system_user_modify.setText(QCoreApplication.translate("MainWindow", u"Modify", None))
        self.btn_system_user_delete.setText(QCoreApplication.translate("MainWindow", u"Delete", None))
        self.label_password2.setText(QCoreApplication.translate("MainWindow", u"Retype Passord", None))
        self.system_user_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_user_group.setText(QCoreApplication.translate("MainWindow", u"Group", None))
        self.label_password.setText(QCoreApplication.translate("MainWindow", u"Password", None))
        self.label_user_enabled.setText(QCoreApplication.translate("MainWindow", u"Enabled", None))
        self.system_user_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.system_user_group.setItemText(0, QCoreApplication.translate("MainWindow", u"Viewer", None))
        self.system_user_group.setItemText(1, QCoreApplication.translate("MainWindow", u"Operator", None))
        self.system_user_group.setItemText(2, QCoreApplication.translate("MainWindow", u"Administrator", None))

        self.system_user_password2.setText("")
        self.system_user_password.setText("")
        self.label_username.setText(QCoreApplication.translate("MainWindow", u"Name", None))
        self.system_tab.setTabText(self.system_tab.indexOf(self.system_user), QCoreApplication.translate("MainWindow", u"User", None))
        self.label_camera_current_datetime.setText(QCoreApplication.translate("MainWindow", u"Current Camera Date and Time", None))
        self.device_datetime.setText(QCoreApplication.translate("MainWindow", u"2020/03/25 11:29/30", None))
        self.label_camera_datetime_set.setText(QCoreApplication.translate("MainWindow", u"New Camera Date and Time", None))
        self.set_datetime_with_pc.setText(QCoreApplication.translate("MainWindow", u"Set this computer's Date and Time", None))
        self.pc_datetime.setText(QCoreApplication.translate("MainWindow", u"2020/03/25 11:29/30", None))
        self.set_datetime_manual.setText(QCoreApplication.translate("MainWindow", u"Set manually", None))
        self.system_datetime_time.setDisplayFormat(QCoreApplication.translate("MainWindow", u"hh:mm:ss", None))
        self.btn_system_datetime_set.setText(QCoreApplication.translate("MainWindow", u"Set Date and Time", None))
        self.label_datetime_configuration.setText(QCoreApplication.translate("MainWindow", u"Configuration", None))
        self.label_ntp_server.setText(QCoreApplication.translate("MainWindow", u"NTP Server", None))
        self.label_timezone.setText(QCoreApplication.translate("MainWindow", u"Time zone", None))
        self.radio_ntp_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.label_enable_ntp.setText(QCoreApplication.translate("MainWindow", u"Enable NTP", None))
        self.radio_ntp_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.label_ntp_sync_time.setText(QCoreApplication.translate("MainWindow", u"Sync time(minute)", None))
        self.ntp_server.setText("")
        self.label_ntp_port.setText(QCoreApplication.translate("MainWindow", u"NTP Port", None))
        self.btn_system_dateteime_apply.setText(QCoreApplication.translate("MainWindow", u"Apply", None))
        self.btn_system_datetime_cancel.setText(QCoreApplication.translate("MainWindow", u"Cancel", None))
        self.system_tab.setTabText(self.system_tab.indexOf(self.system_datetime), QCoreApplication.translate("MainWindow", u"Date Time", None))
        self.btn_system_restart.setText(QCoreApplication.translate("MainWindow", u"Restart", None))
        self.label_restart.setText(QCoreApplication.translate("MainWindow", u"Restart", None))
        self.label_restore_factory_default.setText(QCoreApplication.translate("MainWindow", u"Restore Factory Default", None))
        self.btn_system_restore.setText(QCoreApplication.translate("MainWindow", u"Restore Factory Default", None))
        self.label_maintenance_enable.setText(QCoreApplication.translate("MainWindow", u"Maintenance Day", None))
        self.cksm_fri.setText(QCoreApplication.translate("MainWindow", u"Fir", None))
        self.system_restart_time.setDisplayFormat(QCoreApplication.translate("MainWindow", u"hh:mm:ss", None))
        self.cksm_sun.setText(QCoreApplication.translate("MainWindow", u"Sun", None))
        self.system_maintenance_no.setText(QCoreApplication.translate("MainWindow", u"No", None))
        self.system_maintenance_yes.setText(QCoreApplication.translate("MainWindow", u"Yes", None))
        self.cksm_wed.setText(QCoreApplication.translate("MainWindow", u"Wed", None))
        self.cksm_thu.setText(QCoreApplication.translate("MainWindow", u"Thu", None))
        self.cksm_mon.setText(QCoreApplication.translate("MainWindow", u"Mon", None))
        self.cksm_tue.setText(QCoreApplication.translate("MainWindow", u"Tue", None))
        self.cksm_sat.setText(QCoreApplication.translate("MainWindow", u"Sat", None))
        self.label_maintenace_restart_time.setText(QCoreApplication.translate("MainWindow", u"Restart Time", None))
        self.label_system_maintenance_enable.setText(QCoreApplication.translate("MainWindow", u"Enable", None))
        self.label_system_maintenance_schedule.setText(QCoreApplication.translate("MainWindow", u"System Maintenance Schedule", None))
        self.btn_system_maintenance_apply.setText(QCoreApplication.translate("MainWindow", u"Apply", None))
        self.btn_system_maintenance_cancel.setText(QCoreApplication.translate("MainWindow", u"Cancel", None))
        self.system_tab.setTabText(self.system_tab.indexOf(self.system_maintenance), QCoreApplication.translate("MainWindow", u"Maintenance", None))
        self.label_device_version.setText(QCoreApplication.translate("MainWindow", u"Device Version", None))
        self.label_device_name.setText(QCoreApplication.translate("MainWindow", u"Device Name", None))
        self.label_device_uptime.setText(QCoreApplication.translate("MainWindow", u"Device Uptime", None))
        self.label_device_firmware.setText(QCoreApplication.translate("MainWindow", u"Firmware Version", None))
        self.system_tab.setTabText(self.system_tab.indexOf(self.system_about), QCoreApplication.translate("MainWindow", u"About", None))
        self.main_tab.setTabText(self.main_tab.indexOf(self.system_main), QCoreApplication.translate("MainWindow", u"System", None))
    # retranslateUi

