from essential import *

# {"type":"system_time","settings":{"auto_mode":true,"manual_mode":{"time":1711411471},"ntp_server":{"name":"time.windows.com","port":123,"sync_time":2},"timezone":900}}


class updateData:
    def __init__(self, parent):
        super().__init__()
        self.parent = parent
        # for w in self.parent.sub_tabs :
        #     print (w.objectName())

        self.onesec_timer = QTimer()
        self.onesec_timer.setInterval(1000)

    
    def update(self, p, payload):
        url = "http://%s:%d/api/v1/%s/update" %(param['ip'],param['port'],p)
        r = requests.post(url, headers={'X-Token': param['token']},  json=payload)
        # print (r.text)
        arr= json.loads(r.text)
        return arr

    def update_data_act(self, obj, main_tab, sub_tab):
        # print (obj.objectName(), main_tab, sub_tab)
        btn_name = obj.objectName()
        if main_tab == 'network_main':
            if sub_tab == 'network_ethernet':
                if btn_name == 'btn_network_apply':
                    self.update_network_ethernet()
            elif sub_tab == 'network_udp':
                if btn_name == 'btn_network_apply':
                    self.update_network_udp()
            elif sub_tab == 'network_rtmp':
                if btn_name == 'btn_network_apply':
                    self.update_network_rtmp()

        elif main_tab == 'media_main':
            if sub_tab == 'media_encode':
                if btn_name == 'btn_media_apply':
                    self.update_media_encode()
            elif sub_tab == 'media_video':
                if btn_name == 'btn_media_apply':
                    self.update_media_video()
            elif sub_tab == 'media_audio':
                if btn_name == 'btn_media_apply':
                    self.update_media_audio()
            elif sub_tab == 'media_osd':
                if btn_name == 'btn_media_apply':
                    self.update_media_osd()
            elif sub_tab == 'media_privacy':
                if btn_name == 'btn_media_apply':
                    self.update_media_privacy()
        
        elif main_tab == 'detection_main':
            if sub_tab == 'detect_human':
                if btn_name == 'btn_detection_apply':
                    self.update_detection_human()
            elif sub_tab == 'detect_motion':
                if btn_name == 'btn_detection_apply':
                    self.update_detection_motion()
            elif sub_tab == 'detect_tamper':
                if btn_name == 'btn_detection_apply':
                    self.update_detection_tamper()

        elif main_tab == 'peripherial_main':
            if btn_name == 'btn_detection_apply':
                if sub_tab == 'btn_peripherial_apply':
                    self.update_peri_alram()
            elif sub_tab == 'peri_serial':
                if sub_tab == 'btn_peripherial_apply':
                    self.update_peri_serial()
            elif sub_tab == 'peri_ptz':
                if sub_tab == 'btn_peripherial_apply':
                    self.update_peri_ptz()
            elif sub_tab == 'peri_sd':
                if sub_tab == 'btn_peripherial_apply':
                    self.update_peri_sd()

        elif main_tab == 'system_main':
            if sub_tab == 'system_user':
                self.confirm_user_act(btn_name)
                
            elif sub_tab == 'system_datetime':
                if btn_name == 'btn_system_datetime_set' or btn_name == 'btn_system_dateteime_apply': 
                    self.system_datetime_set()
            elif sub_tab == 'system_maintenance':
                if btn_name == 'btn_system_restart':
                    self.restart_system()
                elif btn_name == 'btn_system_restore':
                    self.restore_system()
                elif btn_name == 'btn_system_maintenance_apply':
                    self.update_system_maintenance()
                elif btn_name == 'btn_system_maintenance_cancel':
                    pass


    def update_network_ethernet(self):
        payload = {
            "type":"ip",
            "settings":{
                "dhcp": self.parent.network_ethernet_dhcp_yes.isChecked(),
                "dns1": check_valid_ipv4(self.parent.dns1.text()),
                "dns2": check_valid_ipv4(self.parent.dns2.text()),
                "ipv4_addr": check_valid_ipv4(self.parent.ipv4_addr.text()),
                "ipv4_gateway": check_valid_ipv4(self.parent.ipv4_gateway.text()),
                "ipv4_mask": check_valid_ipv4(self.parent.ipv4_mask.text()),
                "mac_address": self.parent.mac_address.text(),
                "self_adaption":self.parent.radio_ip_self_yes.isChecked()
            }
        }
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('network', payload)
        if ret['code'] == 20000:
            print ("update success")
        

    def update_network_udp(self):
        print(self.parent)

    def update_network_rtmp(self):
        payload = {
            "type":"rtmp",
            "settings":{
                "app_name": self.parent.app_name.text().strip(),
                "enabled": self.parent.network_ethernet_rtmp_yes.isChecked(),
                "remote_host": self.parent.remote_host.text().strip(),
                "remote_port": self.parent.remote_port.value(),
                "stream_id": self.parent.stream_id.text().strip(),
                "stream_type":self.parent.stream_type.currentIndex()
            }
        }
        
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('network', payload)
        if ret['code'] == 20000:
            print ("update success")


    def update_media_encode(self):
        payload = {
            "type":"video_codec",
            "settings":{
                "main_stream":{
                    "bitrate":int(self.parent.main_stream_bitrate.currentIndex()),
                    "bps":check_int(self.parent.main_stream_bps.text()),
                    "encode_format": int(self.parent.main_stream_encode_format.currentData()),
                    "encode_format_enum":[self.parent.main_stream_encode_format.itemData(i) for i in range(self.parent.main_stream_encode_format.count())],
                    "framerate":check_int(self.parent.main_stream_framerate.text()),
                    "keyframe_interval":check_int(self.parent.main_stream_keyframe_interval.text()),
                    "resolve":int(self.parent.main_stream_resolve.currentData()),
                    "resolve_enum":[self.parent.main_stream_resolve.itemData(i) for i in range(self.parent.main_stream_resolve.count())]
                },
                "sub_stream":{
                    "bitrate":int(self.parent.main_stream_bitrate.currentIndex()),
                    "bps":check_int(self.parent.sub_stream_bps.text()),
                    "encode_format":int(self.parent.sub_stream_encode_format.currentData()),
                    "encode_format_enum":[self.parent.sub_stream_encode_format.itemData(i) for i in range(self.parent.sub_stream_encode_format.count())],
                    "framerate":check_int(self.parent.sub_stream_framerate.text()),
                    "keyframe_interval":check_int(self.parent.sub_stream_keyframe_interval.text()),
                    "resolve":int(self.parent.sub_stream_resolve.currentData()),
                    "resolve_enum":[self.parent.sub_stream_resolve.itemData(i)  for i in range(self.parent.sub_stream_resolve.count())]
                }
            }
        }
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('media', payload)
        if ret['code'] == 20000:
            print ("update success")

        if  (self.parent.place_data.ret['main_stream']['encode_format'] != payload['settings']['main_stream']['encode_format'])  or \
            (self.parent.place_data.ret['sub_stream']['encode_format']  != payload['settings']['sub_stream']['encode_format'])  or \
            (self.parent.place_data.ret['main_stream']['resolve'] != payload['settings']['main_stream']['resolve'])  or \
            (self.parent.place_data.ret['sub_stream']['resolve']  != payload['settings']['sub_stream']['resolve'])  :

            print ("stop video")
            self.parent.scene.timer.stop()
            self.parent.scene.cap.release()
            self.parent.place_data.onesec_timer.stop()            

        ret = self.update('warning', payload)
        if ret['code'] == 20000:
            print ("update success")

        if  (self.parent.place_data.ret['main_stream']['encode_format'] != payload['settings']['main_stream']['encode_format'])  or \
            (self.parent.place_data.ret['sub_stream']['encode_format']  != payload['settings']['sub_stream']['encode_format'])  or \
            (self.parent.place_data.ret['main_stream']['resolve'] != payload['settings']['main_stream']['resolve'])  or \
            (self.parent.place_data.ret['sub_stream']['resolve']  != payload['settings']['sub_stream']['resolve'])  :

            self.parent.place_data.ret['main_stream']['encode_format'] = payload['settings']['main_stream']['encode_format']
            self.parent.place_data.ret['sub_stream']['encode_format']  = payload['settings']['sub_stream']['encode_format']
            self.parent.place_data.ret['main_stream']['resolve'] = payload['settings']['main_stream']['resolve']
            self.parent.place_data.ret['sub_stream']['resolve']  = payload['settings']['sub_stream']['resolve']

            print ("wait for reboot", end =".")
            self.restart_system()


    def update_media_video(self):
        payload = {
            "type":"video_capture",
            "settings":{
                "AE_sensitivity":check_int(self.parent.AE_sensitivity.text()),
                "AE_tolerance":check_int(self.parent.AE_tolerance.text()),
                "backlight":check_int(self.parent.backlight.text()),
                "brightness":check_int(self.parent.brightness.text()),
                "contrast":check_int(self.parent.contrast.text()),
                "horizontal_flip":self.parent.horizontal_flip.isChecked(),
                "noise_reduction_3d":self.parent.noise_reduction_3d.currentIndex(),
                "noise_reduction_3d_enhancement":check_int(self.parent.noise_reduction_3d_enhancement.text()),
                "saturation":check_int(self.parent.saturation.text()),
                "scene_mode":0,
                "sharpness":check_int(self.parent.sharpness.text()),
                "vertical_flip":self.parent.vertical_flip.isChecked(),
                "video_standard":1,
                "wide_dynamic":self.parent.wide_dynamic.currentIndex(),
                "wide_dynamic_enhancement":check_int(self.parent.wide_dynamic_enhancement.text()),
                "prevent_overexposure":False
            }
        }
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('media', payload)
        if ret['code'] == 20000:
            print ("update success")    

    def update_media_audio(self):
        payload = {
            "type":"audio_capture",
            "settings":{
                "collect_volume":check_int(self.parent.collect_volume.text()),
                "input_method":"mic",
                "play_volume":check_int(self.parent.play_volume.text()),
                "sample_bit":16,
                "sample_rate":8000
            }
        }
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('media', payload)
        if ret['code'] == 20000:
            print ("update success")

        payload = {
            "type":"audio_codec",
            "settings":{
                "decode_enabled":True,
                "encode_enabled": self.parent.encode_enabled_yes.isChecked(),
                "encode_format": 4,
                "encode_format_enum":[2,3,4,1]
            }
        }
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('media', payload)
        if ret['code'] == 20000:
            print ("update success")        

    def update_media_osd(self):
        payload = {
            "type":"datetime_title",
            "settings":{
                "datetime":{
                    "enabled": self.parent.osd_datetime_yes.isChecked(),
                    "date_format":self.parent.osd_datetime_date_format.currentIndex(),
                    "time_format":self.parent.osd_datetime_time_format.currentIndex(),
                    "style":1,
                    "week":False,
                    "x":float(self.parent.osd_datetime_x.value()),
                    "y":float(self.parent.osd_datetime_y.value()),
                    "color":"red"
                },
                "title":{
                    "channel":1,
                    "enabled": self.parent.osd_title_yes.isChecked(),
                    "name": str(self.parent.osd_title_name.text()),
                    "x":float(self.parent.osd_title_x.value()),
                    "y":float(self.parent.osd_title_y.value()),
                    "color":"red"
                }
            }
        }
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('media', payload)
        if ret['code'] == 20000:
            print ("update success")        


    def update_media_privacy(self):
        zones = []
        for zone in self.parent.scene.zones:
            zones.append({"color":0, "enabled":zone['enabled'], "height":zone['height'], "width":zone['width'], "x":zone['x'], "y":zone['y'], "fill":False})

        payload = {
            "type":"privacy_mask",
            "settings":{
                "privacy_zone": zones
            }
        }    
        print(payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('media', payload)
        if ret['code'] == 20000:
            print ("update success")        

        self.parent.scene.draw_rectangles(self.parent.media_privacy_graphic_view)

    def update_detection_human(self):
        zones = []
        for zone in self.parent.scene.zones:
            if zone['height'] == 0 and zone['width'] == 0 :
                continue
            zones.append({"height":zone['height'], "width":zone['width'], "x":zone['x'], "y":zone['y']})
        payload = {
            "type":"human_recognition",
            "settings":{
                "alarm_mode":{"alarm_output":False, "capture_numbers":0, "flash_light":False, "play_tone":False, "pre_record_time":0, "record_stream":0, "record_time":0},
                "deployment_area":[],
                "enabled":self.parent.detection_human_enable_yes.isChecked(),
                "path": zones,
                "sensitivity":check_int(self.parent.detection_human_sensitivity.text())
            }
        }
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
       

        if self.parent.place_data.ret['enabled'] != payload['settings']['enabled']:
            print ("stop video")
            self.parent.scene.timer.stop()
            self.parent.scene.cap.release()
            self.parent.place_data.onesec_timer.stop()            

        ret = self.update('warning', payload)
        if ret['code'] == 20000:
            print ("update success")

        if self.parent.place_data.ret['enabled'] != payload['settings']['enabled']:
            self.parent.place_data.ret['enabled'] = payload['settings']['enabled']
            print ("wait for reboot", end =".")
            self.restart_system()
            self.parent.scene.play_video(self.parent.detection_human_graphic_view)


    def update_detection_motion(self):
        path = []
        for j in range(len(self.parent.place_data.rect_item)):
            r_path =[]
            for i in range(len(self.parent.place_data.rect_item[j])):
                r_path.append(1 if self.parent.place_data.rect_item[j][i].isVisible() else 0)
                # print(i,j, self.parent.place_data.rect_item[j][i], self.parent.place_data.rect_item[j][i].isVisible())
            path.append(r_path)
        payload = {
            "type":"motion_detection",
            "settings":{
                "alarm_mode":{"alarm_output":False, "capture_numbers":0, "flash_light":False, "play_tone":False, "pre_record_time":0, "record_stream":0, "record_time":0},
                "deployment_area":[],
                "enabled": self.parent.detection_motion_enable_yes.isChecked(),
                "ratio_enum":[101,202,303,404,1609,1610,2218],
                "rect_lists":path,
                "sensitivity": check_int(self.parent.detection_motion_sensitivity.text())
            }
        }
                    
        print (payload)
        ret = self.update('warning', payload)
        if ret['code'] == 20000:
            print ("update success")        

    def update_detection_tamper(self):
        # sensitivity : 0,25,50,75,100 self.parent.detection_tamper_sensitivity.text()
        payload = {
            "type":"occlusion_detection",
            "settings":{
                "alarm_mode":{"alarm_output":False, "capture_numbers":0, "flash_light":False, "play_tone":False, "pre_record_time":0, "record_stream":0, "record_time":0},
                "deployment_area":[],
                "enabled":self.parent.detection_tamper_enable_yes.isChecked(),
                "path":[{"x":0, "y":0, "width":100, "height":100}],
                "sensitivity": int(round(int(self.parent.detection_tamper_sensitivity.text())/25) *25)
            }
        }

        print(payload)
        # if not check_payload(payload['settings']):
        #     print ("ERROR")
        #     return False
        ret = self.update('warning', payload)
        print (ret)
        if ret['code'] == 20000:
            print ("update success")     



    def update_peri_alram(self):
        pass
    def update_peri_serial(self):
        pass
    def update_peri_ptz(self):
        pass
    def update_peri_sd(self):
        pass
    
    def update_system_user(self, mode):
        if len(self.parent.system_user_name.text().strip()) < 4 :
            return True, "Username Length Error"

        if len(self.parent.system_user_password.text().strip()) <6 :
            return True, "Password Length Error"
        
        if not (self.parent.system_user_password.text().strip() == self.parent.system_user_password2.text().strip()):
            return True, "Password Error"

        payload = {
            "type":"type_user",
            "settings":{
                "name":     self.parent.system_user_name.text().strip(),
                "password": self.parent.system_user_password.text().strip(),
                "classify": self.parent.system_user_group.currentIndex(),
                "enabled":  self.parent.system_user_yes.isChecked()
            }
        }
        if mode == 'add':
            payload['type'] = "add_user"
        elif mode == 'modify':
            payload['type'] = "modify_user"
        elif mode == 'delete':
            payload['type'] = "remove_user"
                
        print (payload)
        if not check_payload(payload['settings']):
            print ("ERROR")
            return False
        ret = self.update('system', payload)
        if ret['code'] == 20000:
            return False # No error
        
    
    def confirm_user_act(self, btn_name):
        if btn_name == 'btn_system_user_add':
            ret = self.update_system_user('add')
            QMessageBox.warning(self.parent, 'Error', ret[1]) if ret  else  self.parent.place_data.view_system_user()

        elif btn_name == 'btn_system_user_modify':
            ret = self.update_system_user('modify')
            QMessageBox.warning(self.parent, 'Error', ret[1]) if ret  else  self.parent.place_data.view_system_user()

        elif btn_name == 'btn_system_user_delete':
            reply = QMessageBox.question(self.parent, 'Confirm Delete', 'Are you sure to delete?', QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if reply == QMessageBox.Yes:
                self.parent.system_user_password.setText('hellothisishanskim')
                self.parent.system_user_password2.setText('hellothisishanskim')
                ret = self.update_system_user('delete')
                QMessageBox.warning(self.parent, 'Error', ret[1]) if ret  else  self.parent.place_data.view_system_user()
            else:
                print ("No")


    def system_datetime_set(self):
        ts = int(time.time())
        if self.parent.set_datetime_manual.isChecked():
            datetime_str = self.parent.system_datetime_date.text() + " " + self.parent.system_datetime_time.text()
            tm = time.strptime(datetime_str, '%Y-%m-%d %H:%M:%S')
            ts = int(time.mktime(tm))
        payload = {
            "type":"system_time",
            "settings":{
                "auto_mode":self.parent.radio_ntp_yes.isChecked(),
                "manual_mode":{"time":ts},
                "ntp_server":{
                    "name": self.parent.ntp_server.text().strip(),
                    "port": self.parent.ntp_port.value(),
                    "sync_time": self.parent.sync_time.value()
                },
                "timezone":self.parent.timezone.currentData()
            }
        }
        print (payload)
        ret = self.update('system', payload)
        if ret['code'] == 20000:
            print ("update success")
        self.parent.place_data.view_system_datetime()

    def restart_system(self):
        self.parent.scene.timer.stop()
        self.parent.scene.cap.release()
        self.parent.place_data.onesec_timer.stop()

        if self.parent.main_objname == 'system_main' and self.parent.sub_objname[self.parent.main_objname] == 'system_maintenance':
            url = "http://%s:%d/api/v1/system/restart" %(param['ip'],param['port'])
            r = requests.get(url, headers={'X-Token': param['token']})
            # print (r.text)

        msg = QMessageBox()
        msg.setWindowTitle("Reboot")
        msg.setStandardButtons(QMessageBox.NoButton)
        msg.setText('     Rebooting  ( 20 )     ')

        try:
            self.onesec_timer.timeout.disconnect()
        except:
            pass

        self.onesec_timer.start()
        self.i = 30
        def ppp():
            msg.setText("     Rebooting  ( %d )     " %self.i)
            if 3  < self.i < 25:
                if is_online(param['ip'], int(param['port'])):
                    self.i = 3
            print (self.i)
            if self.i == 0:   # after 2 second 
                msg.accept()
                msg.close()
            self.i -= 1

        self.onesec_timer.timeout.connect(ppp)
        msg.exec_()
        self.onesec_timer.stop()

        self.parent.scene.init_cam()
        self.parent.scene.timer.start()
        self.parent.scene.play = False
        self.parent.place_data.onesec_timer.start()

    def restore_system(self):
        pass

    def update_system_maintenance(self):
        day_week = [self.parent.cksm_sun, self.parent.cksm_mon, self.parent.cksm_tue, self.parent.cksm_wed, self.parent.cksm_thu, self.parent.cksm_fri, self.parent.cksm_sat]
        days = 0
        for x, day_check in enumerate(day_week):
            days |= (day_check.isChecked() << x)

        payload = {
            "type":"maintenance",
            "settings":{
                "day":days,
                "enabled":self.parent.system_maintenance_yes.isChecked(),
                "restart_time": self.parent.system_restart_time.text()
            }
        }
        print (payload)
        ret = self.update('system', payload)
        if ret['code'] == 20000:
            print ("update success")        


# class PopupReboot(QDialog):
#     def __init__(self):
#         super().__init__()
#         self.ui = uic.loadUi("Camera\pop_reboot.ui")
#         self.show()



# def update(p, payload):
#     url = "http://%s:%d/api/v1/%s/update" %(param['ip'],param['port'],p)
#     r = requests.post(url, headers={'X-Token': param['token']},  json=payload)

#     # print (r.text)
#     arr= json.loads(r.text)
#     return arr


def check_valid_ipv4(ip_addr):
    ip_ex = ip_addr.split(".")
    ip_arr = []
    if len(ip_ex) !=4:
        return "ERROR"
    for n in ip_ex:
        try:
            n = int(n)
        except:
            return "ERROR"
        if n <0 or n > 255:
            return "ERROR"
        ip_arr.append(n)

    return "%d.%d.%d.%d" %tuple(ip_arr)

def check_int(num):
    try:
        n = int(num)
    except:
        return "ERROR"
    return n

def check_payload(pl):
    for x in pl:
        if type(pl[x]) == str and "ERROR" in pl[x]: 
            print (x, "ERROR")
            return False
    return True

# def update_data_act(s, main_tab, sub_tab):
#     print (s.objectName(), main_tab, sub_tab)
    # print (s.sub_objname[s.main_index])
    # if s.main_objname == 'network_main':
    #     if s.sub_objname[s.main_index] == 'network_ethernet':
    #         update_network_ethernet(s)
    #     elif s.sub_objname[s.main_index] == 'network_udp':
    #         update_network_udp(s)
    #     elif s.sub_objname[s.main_index] == 'network_rtmp':
    #         update_network_rtmp(s)

    # elif s.main_objname == 'media_main':
    #     if s.sub_objname[s.main_index] == 'media_encode':
    #         update_media_encode(s)
    #     elif s.sub_objname[s.main_index] == 'media_video':
    #         update_media_video(s)
    #     elif s.sub_objname[s.main_index] == 'media_audio':
    #         update_media_audio(s)
    #     elif s.sub_objname[s.main_index] == 'media_osd':
    #         update_media_osd(s)
    #     elif s.sub_objname[s.main_index] == 'media_privacy':
    #         update_media_privacy(s)
    
    # elif s.main_objname == 'detection_main':
    #     if s.sub_objname[s.main_index] == 'detect_human':
    #         update_detection_human(s)
    #     elif s.sub_objname[s.main_index] == 'detect_motion':
    #         update_detection_motion(s)
    #     elif s.sub_objname[s.main_index] == 'detect_tamper':
    #         update_detection_tamper(s)



# def update_network_ethernet(s):
#     print (s)

#     payload = {
#         "type":"ip",
#         "settings":{
#             "dhcp": s.network_ethernet_dhcp_yes.isChecked(),
#             "dns1": check_valid_ipv4(s.dns1.text()),
#             "dns2": check_valid_ipv4(s.dns2.text()),
#             "ipv4_addr": check_valid_ipv4(s.ipv4_addr.text()),
#             "ipv4_gateway": check_valid_ipv4(s.ipv4_gateway.text()),
#             "ipv4_mask": check_valid_ipv4(s.ipv4_mask.text()),
#             "mac_address": s.mac_address.text(),
#             "self_adaption":s.radio_ip_self_yes.isChecked()
#         }
#     }
#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('network', payload)
#     if ret['code'] == 20000:
#         print ("update success")
    

# def update_network_udp(s):
#     print(s)

# def update_network_rtmp(s):
#     if s.network_ethernet_rtmp_yes.isChecked():
#         payload = {
#             "type":"rtmp",
#             "settings":{
#                 "app_name": s.app_name.text(),
#                 "enabled": True ,
#                 "remote_host": check_valid_ipv4(s.remote_host.text()),
#                 "remote_port": int(s.remote_port.text()),
#                 "stream_id": s.stream_id.text(),
#                 "stream_type":s.stream_type.currentIndex()
#             }
#         }
#     else :
#         payload = {
#             "type":"rtmp", 
#             "settings":{
#                 "app_name": s.app_name.text(),
#                 "enabled": False ,
#                 "remote_host": s.remote_host.text(),
#                 "remote_port": int(s.remote_port.text()),
#                 "stream_id": s.stream_id.text(),
#                 "stream_type":s.stream_type.currentIndex()
#             } 
#         }
    
#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('network', payload)
#     if ret['code'] == 20000:
#         print ("update success")

# def update_media_encode(s):
# # {'type': 'video_codec', 'settings': {'main_stream': {'bitrate': 1, 'bps': 6000, 'encode_format': 3, 'encode_format_enum': [2,3], 'framerate': 25, 'keyframe_interval': 50, 'resolve': 28801620, 'resolve_enum': [12800720, 12800960, 19201080, 23041296, 25601440, 26881520, 28801620]}, 'sub_stream': {'bitrate': 1, 'bps': 512, 'encode_format': 2, 'encode_format_enum': [2, 3], 'framerate': 25, 'keyframe_interval': 50, 'resolve': 6400360, 'resolve_enum': [3200240, 3520288, 6400360, 6400480, 7200576, 12800720]}}}    
# # {"type": "video_codec", "settings": {"main_stream": {"bitrate": 1, "bps": 6000, "encode_format": 3, "encode_format_enum": [2,3], "framerate": 25, "keyframe_interval": 50, "resolve": 28801620, "resolve_enum": [12800720, 12800960, 19201080, 23041296, 25601440, 26881520, 28801620]}, "sub_stream": {"bitrate": 1, "bps": 512, "encode_format": 2, "encode_format_enum": [2, 3], "framerate": 25, "keyframe_interval": 50, "resolve": 6400360, "resolve_enum": [3200240, 3520288, 6400360, 6400480, 7200576, 12800720]}}}

#     payload = {
#         "type":"video_codec",
#         "settings":{
#             "main_stream":{
#                 "bitrate":int(s.main_stream_bitrate.currentIndex()),
#                 "bps":check_int(s.main_stream_bps.text()),
#                 "encode_format": int(s.main_stream_encode_format.currentData()),
#                 "encode_format_enum":[s.main_stream_encode_format.itemData(i) for i in range(s.main_stream_encode_format.count())],
#                 "framerate":check_int(s.main_stream_framerate.text()),
#                 "keyframe_interval":check_int(s.main_stream_keyframe_interval.text()),
#                 "resolve":int(s.main_stream_resolve.currentData()),
#                 "resolve_enum":[s.main_stream_resolve.itemData(i) for i in range(s.main_stream_resolve.count())]
#             },
#             "sub_stream":{
#                 "bitrate":int(s.main_stream_bitrate.currentIndex()),
#                 "bps":check_int(s.sub_stream_bps.text()),
#                 "encode_format":int(s.sub_stream_encode_format.currentData()),
#                 "encode_format_enum":[s.sub_stream_encode_format.itemData(i) for i in range(s.sub_stream_encode_format.count())],
#                 "framerate":check_int(s.sub_stream_framerate.text()),
#                 "keyframe_interval":check_int(s.sub_stream_keyframe_interval.text()),
#                 "resolve":int(s.sub_stream_resolve.currentData()),
#                 "resolve_enum":[s.sub_stream_resolve.itemData(i)  for i in range(s.sub_stream_resolve.count())]
#             }
#         }
#     }
#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('media', payload)
#     if ret['code'] == 20000:
#         print ("update success")

# def update_media_video(s):
# # {"type":"video_capture","settings":{"AE_sensitivity":128,"AE_tolerance":128,"backlight":50,"brightness":49,"contrast":51,"horizontal_flip":false,"noise_reduction_3d":2,"noise_reduction_3d_enhancement":0,"saturation":51,"scene_mode":2,"sharpness":50,"vertical_flip":false,"video_standard":0,"wide_dynamic":1,"wide_dynamic_enhancement":0}}
# # {'type':'video_capture','settings':{'AE_sensitivity':128,'AE_tolerance':128,'backlight':50,'brightness':49,'contrast':51,'horizontal_flip':False,'noise_reduction_3d':2,'noise_reduction_3d_enhancement':0,'saturation':51,'scene_mode':0,'sharpness':50,'vertical_flip':False,'video_standard':1,'wide_dynamic':1,'wide_dynamic_enhancement':0,'prevent_overexposure': False}}
#     payload = {
#         "type":"video_capture",
#         "settings":{
#             "AE_sensitivity":check_int(s.AE_sensitivity.text()),
#             "AE_tolerance":check_int(s.AE_tolerance.text()),
#             "backlight":check_int(s.backlight.text()),
#             "brightness":check_int(s.brightness.text()),
#             "contrast":check_int(s.contrast.text()),
#             "horizontal_flip":s.horizontal_flip.isChecked(),
#             "noise_reduction_3d":s.noise_reduction_3d.currentIndex(),
#             "noise_reduction_3d_enhancement":check_int(s.noise_reduction_3d_enhancement.text()),
#             "saturation":check_int(s.saturation.text()),
#             "scene_mode":0,
#             "sharpness":check_int(s.sharpness.text()),
#             "vertical_flip":s.vertical_flip.isChecked(),
#             "video_standard":1,
#             "wide_dynamic":s.wide_dynamic.currentIndex(),
#             "wide_dynamic_enhancement":check_int(s.wide_dynamic_enhancement.text()),
#             "prevent_overexposure":False
#         }
#     }
#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('media', payload)
#     if ret['code'] == 20000:
#         print ("update success")    

# def update_media_audio(s):
#     payload = {
#         "type":"audio_capture",
#         "settings":{
#             "collect_volume":check_int(s.collect_volume.text()),
#             "input_method":"mic",
#             "play_volume":check_int(s.play_volume.text()),
#             "sample_bit":16,
#             "sample_rate":8000
#         }
#     }
#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('media', payload)
#     if ret['code'] == 20000:
#         print ("update success")

#     payload = {
#         "type":"audio_codec",
#         "settings":{
#             "decode_enabled":True,
#             "encode_enabled":True if s.encode_enabled_yes.isChecked() else False,
#             "encode_format":4,
#             "encode_format_enum":[2,3,4,1]
#         }
#     }
#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('media', payload)
#     if ret['code'] == 20000:
#         print ("update success")        

# def update_media_osd(s):
#     payload = {
#         "type":"datetime_title",
#         "settings":{
#             "datetime":{
#                 "date_format":s.osd_datetime_date_format.currentIndex(),
#                 "enabled": s.osd_datetime_yes.isChecked(),
#                 "style":0,
#                 "time_format":s.osd_datetime_time_format.currentIndex(),
#                 "week":False,
#                 "x":float(s.osd_datetime_x.value()),
#                 "y":float(s.osd_datetime_y.value()),
#                 "color":"red"
#             },
#             "title":{
#                 "channel":1,
#                 "enabled": s.osd_title_yes.isChecked(),
#                 "name": str(s.osd_title_name.text()),
#                 "x":float(s.osd_title_x.value()),
#                 "y":float(s.osd_title_y.value()),
#                 "color":"red"
#             }
#         }
#     }
#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('media', payload)
#     if ret['code'] == 20000:
#         print ("update success")        


# def update_media_privacy(s):
#     zones = []
#     for zone in s.scene.zones:
#         zones.append({"color":0, "enabled":zone['enabled'], "height":zone['height'], "width":zone['width'], "x":zone['x'], "y":zone['y'], "fill":False})

#     payload = {
#         "type":"privacy_mask",
#         "settings":{
#             "privacy_zone": zones
#         }
#     }    
#     print(payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('media', payload)
#     if ret['code'] == 20000:
#         print ("update success")        

#     s.scene.draw_rectangles(s.media_privacy_graphic_view)



# def update_detection_human(s):
#     zones = []
#     for zone in s.scene.zones:
#         if zone['height'] == 0 and zone['width'] == 0 :
#             continue
#         zones.append({"height":zone['height'], "width":zone['width'], "x":zone['x'], "y":zone['y']})
#     payload = {
#         "type":"human_recognition",
#         "settings":{
#             "alarm_mode":{
#                 "alarm_output":False,
#                 "capture_numbers":0,
#                 "flash_light":False,
#                 "play_tone":False,
#                 "pre_record_time":0,
#                 "record_stream":0,
#                 "record_time":0
#             },
#             "deployment_area":[],
#             "enabled":True,
#             "path":zones,
#             "sensitivity":63
#         }
#     }
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('warning', payload)
#     if ret['code'] == 20000:
#         print ("update success")        


# def update_detection_motion(s):
#     payload = {
#         "type":"motion_detection",
#         "settings":{
#             "alarm_mode":{
#                 "alarm_output":False,
#                 "capture_numbers":0,
#                 "flash_light":False,
#                 "play_tone":False,
#                 "pre_record_time":0,
#                 "record_stream":0,
#                 "record_time":0
#             },
#             "deployment_area":[],
#             "enabled": s.detection_motion_enable_yes.isChecked(),
#             "ratio_enum":[101,202,303,404,1609,1610,2218],
#             "rect_lists":[
#                 [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
#                 [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
#                 [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
#                 [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0]
#             ],
#             "sensitivity": check_int(s.detection_motion_sensitivity.text())
#         }
#     }
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('warning', payload)
#     if ret['code'] == 20000:
#         print ("update success")        


# def update_detection_tamper(s):
#     payload = {
#         "type":"occlusion_detection",
#         "settings":{
#             "alarm_mode":{
#                 "alarm_output":False,
#                 "capture_numbers":0,
#                 "flash_light":False,
#                 "play_tone":False,
#                 "pre_record_time":0,
#                 "record_stream":0,
#                 "record_time":0
#             },
#             "deployment_area":[],
#             "enabled":s.detection_tamper_enable_yes.isChecked(),
#             "path":[
#                 {"height":100,"width":100,"x":0,"y":0}
#             ],
#             "sensitivity":check_int(s.detection_tamper_sensitivity.text())
#         }
#     }
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('warning', payload)
#     if ret['code'] == 20000:
#         print ("update success")     





# def update_system_user(s, mode):
#     payload = {}
#     if len(s.system_user_name.text().strip()) < 4 :
#         return True, "Username Length Error"

#     if len(s.system_user_password.text().strip()) <6 :
#         return True, "Password Length Error"

#     if not (s.system_user_name.text().strip() == s.system_user_name_old.text().strip()):
#         return True, "Name Error"
    
#     if not (s.system_user_password.text().strip() == s.system_user_password2.text().strip()):
#         return True, "Password Error"

#     if mode == 'add':
#         payload = {
#             "type":"add_user",
#             "settings":{
#                 "name": s.system_user_name.text().strip(),
#                 "password":s.system_user_password.text().strip(),
#                 "classify": s.system_user_group.currentIndex(),
#                 "enabled": s.system_user_yes.isChecked()
#             }
#         }

#     elif mode == 'modify':
#         payload = {
#             "type":"modify_user",
#             "settings":{
#                 "classify":s.system_user_group.currentIndex(),
#                 "enabled":s.system_user_yes.isChecked(),
#                 "name": s.system_user_name.text().strip(),
#                 "password": s.system_user_password.text().strip()
#             }
#         }
#     elif mode == 'delete':
#         payload = {
#             "type":"remove_user",
#             "settings":{
#                 "classify":s.system_user_group.currentIndex(),
#                 "enabled":s.system_user_yes.isChecked(),
#                 "name": s.system_user_name.text().strip(),
#                 "password": s.system_user_password.text().strip()
#             }
#         }

    

#     print (payload)
#     if not check_payload(payload['settings']):
#         print ("ERROR")
#         return False
#     ret = update('system', payload)
#     if ret['code'] == 20000:
#         return False # No error
    
#     return True, "Server Error"