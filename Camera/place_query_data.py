
from essential import *

class placeQueryData:
    def __init__(self, parent):
        super().__init__()
        self.parent = parent
        # for w in self.parent.sub_tabs :
        #     print (w.objectName())

        # self.connect_actions()

        self.connect_flags = {}
        self.onesec_timer = QTimer(self.parent)
        self.onesec_timer.setInterval(1000)
        self.onesec_timer.start()

        param['token'] = self.getToken()
        self.ret = {}


    def getToken(self) :
        r = requests.post("http://%s:%d%s" %(param['ip'], param['port'], cgi_login), json={"username":param['username'],"password":param['password']})
        arr = json.loads(r.text)
        if arr['code'] == 20000:
            return arr['data']['token']
        else:
            return arr

    def query(self, p, q):
        url = "http://%s:%d/api/v1/%s/settings?query=%s" %(param['ip'],param['port'],p,q)
        r = requests.get(url, headers={'X-Token': param['token']})

        # print (r.text)
        arr= json.loads(r.text)
        if arr['code'] == 50008:
            #  {"code":50008,"message":"Account and password are incorrect."}, refresh token    
            param['token'] = getToken(param)
            url = "http://%s:%d/api/v1/%s/settings?query=%s" %(param['ip'], param['port'],p,q)
            print (url)
            r = requests.get(url, headers={'X-Token': param['token']})
            arr= json.loads(r.text)

        if arr['code'] == 20000 and  arr['data'] and arr['data'].get('settings'):
                return arr['data']['settings']
        else:
            return arr    
        
    def view_query_act(self):
        main_objname = self.parent.main_objname
        sub_objname  = self.parent.sub_objname[self.parent.main_objname]
        # print (main_objname, sub_objname)

        if main_objname == 'network_main':
            if sub_objname == 'network_ethernet':
                self.view_network_ethernet()
            elif sub_objname == 'network_udp':
                self.view_network_udp()
            elif sub_objname == 'network_rtmp':
                self.view_network_rtmp()
        
        elif main_objname == 'media_main' :
            if sub_objname == 'media_encode':
                self.view_media_encode()
            elif sub_objname == 'media_video':
                self.view_media_video()
            elif sub_objname == 'media_audio':
                self.view_media_audio()
            elif sub_objname == 'media_osd':
                self.view_media_osd()
            elif sub_objname == 'media_privacy':
                self.view_media_privacy()
                
        elif main_objname == 'detection_main' :
            if sub_objname == 'detect_human':
                self.view_detection_human()
            elif sub_objname == 'detect_motion':
                self.view_detection_motion()
            elif sub_objname == 'detect_tamper':
                self.view_detection_tamper()

        elif main_objname == 'peripherial' :
            if sub_objname == 'peri_alarm':
                self.view_peripherial_alarm()
            elif sub_objname == 'peri_serial':
                self.view_peripherial_serial()
            elif sub_objname == 'peri_ptz':
                self.view_peripherial_ptz()
            elif sub_objname == 'peri_sd':
                self.view_peripherial_sd()
            
        elif main_objname == 'system_main' :
            if sub_objname == 'system_user':
                self.view_system_user()
            elif sub_objname == 'system_datetime':
                self.view_system_datetime()
            elif sub_objname == 'system_maintenance':
                self.view_system_maintenance()
            elif sub_objname == 'system_about':
                self.view_system_about()

    def goSlider(self, text_tag, slider_tag):
        try:
            val = int(text_tag.text())
        except :
            return False
        if val > slider_tag.maximum() :
            val = slider_tag.maximum()
        elif val < slider_tag.minimum() :
            val = slider_tag.minimum()
        text_tag.setText(str(val))
        slider_tag.setValue(val)
            
    def linkTextSlider(self, text_tag, slider_tag, val):
        text_tag.setText(str(val))
        slider_tag.setValue(val)
        if not self.connect_flags.get(text_tag.objectName()):
            text_tag.editingFinished.connect(lambda: self.goSlider(text_tag, slider_tag))
            slider_tag.valueChanged[int].connect(lambda: text_tag.setText(str(slider_tag.value())))
            self.connect_flags[text_tag.objectName()] = True
            print ("text_tag ", text_tag.objectName(), " connected")

    def view_network_ethernet(self):
        self.ret = self.query('network', 'ip')
        self.parent.mac_address.setText(self.ret['mac_address'])
        self.parent.network_ethernet_dhcp_yes.setChecked(True) if self.ret['dhcp'] else self.parent.network_ethernet_dhcp_no.setChecked(True)
        self.parent.ipv4_addr.setText(self.ret['ipv4_addr'])
        self.parent.ipv4_mask.setText(self.ret['ipv4_mask'])
        self.parent.ipv4_gateway.setText(self.ret['ipv4_gateway'])
        self.parent.dns1.setText(self.ret['dns1'])
        self.parent.dns2.setText(self.ret['dns2'])
        self.parent.radio_ip_self_yes.setChecked(True) if self.ret['self_adaption'] else self.parent.radio_ip_self_no.setChecked(True)

    def view_network_udp(self):
        self.ret = self.query('network', 'udp')
        self.parent.network_ethernet_udp_no.setChecked(True)
        self.parent.network_ethernet_udp_yes.setChecked(True) if self.ret['auto_mode'] == True else self.parent.network_ethernet_udp_no.setChecked(True)


    def rtmp_play_address_set(self):
        self.parent.rtmp_play_address.setText("rtmp://%s:%d/%s/%s" %(self.parent.remote_host.text(), int(self.parent.remote_port.text()), self.parent.app_name.text(), self.parent.stream_id.text()))

    def view_network_rtmp(self):
        self.ret = self.query('network', 'rtmp')
        
        self.parent.network_ethernet_rtmp_yes.setChecked(True) if self.ret['enabled'] == True else self.parent.network_ethernet_rtmp_no.setChecked(True)
        self.parent.remote_host.setText(self.ret['remote_host'])
        self.parent.remote_port.setValue(self.ret['remote_port'])
        self.parent.app_name.setText(self.ret['app_name'])
        self.parent.stream_id.setText(self.ret['stream_id'])
        self.parent.stream_type.setCurrentIndex(int(self.ret['stream_type']))
        self.rtmp_play_address_set()
        if not self.connect_flags.get('rtmp'):
            self.parent.remote_host.editingFinished.connect(self.rtmp_play_address_set)
            self.parent.remote_port.editingFinished.connect(self.rtmp_play_address_set)
            self.parent.app_name.editingFinished.connect(self.rtmp_play_address_set)
            self.parent.stream_id.editingFinished.connect(self.rtmp_play_address_set)      
            self.connect_flags['rtmp'] = True

    def view_media_encode(self):
        self.ret = self.query('media', 'video_codec')
        self.parent.main_stream_encode_format.clear()
        for e in self.ret['main_stream']['encode_format_enum']:
            if e == 2:
                self.parent.main_stream_encode_format.addItem('H.264',2)
            elif e == 3:
                self.parent.main_stream_encode_format.addItem('H.265',3)

        for i, e in  enumerate([self.parent.main_stream_encode_format.itemData(x) for x in range(self.parent.main_stream_encode_format.count())]):
            if e == self.ret['main_stream']['encode_format']:
                self.parent.main_stream_encode_format.setCurrentIndex(i)
                break
        
        self.parent.main_stream_resolve.clear()
        for i, e in enumerate(self.ret['main_stream']['resolve_enum']):
            x = e //10000
            y = e-x*10000
            item = "%dx%d" %(x,y)
            self.parent.main_stream_resolve.addItem(item, e)
            if e == self.ret['main_stream']['resolve']:
                self.parent.main_stream_resolve.setCurrentIndex(i)
     
        self.parent.main_stream_bitrate.setCurrentIndex(self.ret['main_stream']['bitrate'])

        self.linkTextSlider(self.parent.main_stream_framerate, self.parent.main_stream_framerate_slider, self.ret['main_stream']['framerate'])
        self.linkTextSlider(self.parent.main_stream_bps, self.parent.main_stream_bps_slider, self.ret['main_stream']['bps'])
        self.linkTextSlider(self.parent.main_stream_keyframe_interval, self.parent.main_stream_keyframe_interval_slider, self.ret['main_stream']['keyframe_interval'])

        self.parent.sub_stream_encode_format.clear()
        for e in self.ret['sub_stream']['encode_format_enum']:
            if e == 2:
                self.parent.sub_stream_encode_format.addItem('H.264',2)
            elif e == 3:
                self.parent.sub_stream_encode_format.addItem('H.265',3)
                
        for i, e in  enumerate([self.parent.sub_stream_encode_format.itemData(x) for x in range(self.parent.sub_stream_encode_format.count())]):
            if e == self.ret['sub_stream']['encode_format']:
                self.parent.sub_stream_encode_format.setCurrentIndex(i)
                break

        self.parent.sub_stream_resolve.clear()
        for i, e in enumerate(self.ret['sub_stream']['resolve_enum']):
            x = e //10000
            y = e-x*10000
            item = "%dx%d" %(x,y)
            self.parent.sub_stream_resolve.addItem(item, e)
            if e == self.ret['sub_stream']['resolve']:
                self.parent.sub_stream_resolve.setCurrentIndex(i)
        
        self.parent.sub_stream_bitrate.setCurrentIndex(self.ret['sub_stream']['bitrate'])
        self.linkTextSlider(self.parent.sub_stream_framerate, self.parent.sub_stream_framerate_slider, self.ret['sub_stream']['framerate'])
        self.linkTextSlider(self.parent.sub_stream_bps, self.parent.sub_stream_bps_slider, self.ret['sub_stream']['bps'])
        self.linkTextSlider(self.parent.sub_stream_keyframe_interval, self.parent.sub_stream_keyframe_interval_slider, self.ret['sub_stream']['keyframe_interval'])

    def view_media_video(self):
        self.parent.scene.play_video(self.parent.media_video_graphic_view)
        self.ret = self.query('media', 'video_capture')
        self.linkTextSlider(self.parent.brightness, self.parent.brightness_slider, self.ret['brightness'])
        self.linkTextSlider(self.parent.contrast, self.parent.contrast_slider, self.ret['contrast'])
        self.linkTextSlider(self.parent.saturation, self.parent.saturation_slider, self.ret['saturation'])
        self.linkTextSlider(self.parent.backlight, self.parent.backlight_slider, self.ret['backlight'])
        self.linkTextSlider(self.parent.sharpness, self.parent.sharpness_slider, self.ret['sharpness'])
        if self.ret['horizontal_flip']:
            self.parent.horizontal_flip.setChecked(True)
        if self.ret['vertical_flip']:
            self.parent.vertical_flip.setChecked(True)
        self.linkTextSlider(self.parent.AE_sensitivity, self.parent.AE_sensitivity_slider, self.ret['AE_sensitivity'])
        self.linkTextSlider(self.parent.AE_tolerance, self.parent.AE_tolerance_slider, self.ret['AE_tolerance'])
        self.parent.wide_dynamic.setCurrentIndex(int(self.ret['wide_dynamic']))
        self.parent.noise_reduction_3d.setCurrentIndex(int(self.ret['noise_reduction_3d']))
        self.linkTextSlider(self.parent.wide_dynamic_enhancement, self.parent.wide_dynamic_enhancement_slider, self.ret['wide_dynamic_enhancement'])
        self.linkTextSlider(self.parent.noise_reduction_3d_enhancement, self.parent.noise_reduction_3d_enhancement_slider, self.ret['noise_reduction_3d_enhancement'])


    def view_media_audio(self):
        self.ret = self.query('media', 'audio_capture')
        self.ret.update(self.query('media', 'audio_codec'))    
        self.parent.sample_rate.setCurrentIndex(0)
        self.parent.sample_bit.setCurrentIndex(0)
        self.linkTextSlider(self.parent.collect_volume, self.parent.collect_volume_slider, self.ret['collect_volume'])
        self.linkTextSlider(self.parent.play_volume, self.parent.play_volume_slider, self.ret['play_volume'])
        self.parent.audio_encode_enabled_yes.setChecked(True) if self.ret['encode_enabled'] else self.parent.audio_encode_enabled_no.setChecked(True)


    def view_media_osd(self):
        self.parent.scene.play_video(self.parent.media_osd_graphic_view)
        self.ret = self.query('media', 'datetime_title')
        self.parent.osd_datetime_yes.setChecked(True) if self.ret['datetime']['enabled'] else  self.parent.osd_datetime_no.setChecked(True)
        self.parent.osd_datetime_x.setValue(self.ret['datetime']['x'])
        self.parent.osd_datetime_y.setValue(self.ret['datetime']['y'])
        self.parent.osd_datetime_time_format.setCurrentIndex(self.ret['datetime']['time_format'])
        self.parent.osd_datetime_date_format.setCurrentIndex(self.ret['datetime']['date_format'])

        self.parent.osd_title_yes.setChecked(True) if self.ret['title']['enabled'] else self.parent.osd_title_no.setChecked(True)
        self.parent.osd_title_x.setValue(self.ret['title']['x'])
        self.parent.osd_title_y.setValue(self.ret['title']['y'])
        self.parent.osd_title_name.setText(str(self.ret['title']['name']))

    def view_media_privacy(self):
        radio_list = [self.parent.privacy_zone0_sel, self.parent.privacy_zone1_sel, self.parent.privacy_zone2_sel, self.parent.privacy_zone3_sel] 
        check_list = [self.parent.privacy_zone0_enabled, self.parent.privacy_zone1_enabled, self.parent.privacy_zone2_enabled, self.parent.privacy_zone3_enabled]

        def link_zone_sel():
            for i, radio in enumerate(radio_list):
                if radio.isChecked():
                    self.parent.scene.current_zone = i
                    break
            for i, check in enumerate(check_list):
                self.parent.scene.zones[i]['enabled'] = check.isChecked()
                self.parent.scene.zones[i]['obj'].show() if check.isChecked() else  self.parent.scene.zones[i]['obj'].hide()
            
        
        if not self.connect_flags.get('privacy_mask'):
            for rc  in (radio_list + check_list):
                rc.clicked.connect(link_zone_sel)
                self.connect_flags['privacy_mask'] = True
            print ("privacy_mask connected")

        link_zone_sel()

        self.ret = self.query('media', 'privacy_mask')
        colors = [Qt.red, Qt.yellow, Qt.green, Qt.blue]
        for i, zone in enumerate(self.ret['privacy_zone']):
            self.parent.scene.zones[i].update(zone)
            self.parent.scene.zones[i]['color'] = colors[i] 

        for i, check in enumerate(check_list):
            self.parent.scene.zones[i]['enabled'] = self.ret['privacy_zone'][i]['enabled']
            check.setChecked(True) if self.ret['privacy_zone'][i]['enabled'] else check.setChecked(False)
            
        self.parent.scene.play_video(self.parent.media_privacy_graphic_view)
        self.parent.scene.draw_rectangles(self.parent.media_privacy_graphic_view)
        # for z in self.parent.scene.zones:
        #     print (z)

    def view_detection_human(self):
        radio_list = [self.parent.radio_detect_human_zone0, self.parent.radio_detect_human_zone1, self.parent.radio_detect_human_zone2, self.parent.radio_detect_human_zone3] 
        def link_zone_sel():
            for i, radio in enumerate(radio_list):
                if radio.isChecked():
                    self.parent.scene.current_zone = i
                    break
        if not self.connect_flags.get('detection_human'):
            for rc  in radio_list:
                rc.clicked.connect(link_zone_sel)
            self.connect_flags['detection_human'] = True
            print ("detection_human connected")

        link_zone_sel()
        self.ret = self.query('warning', 'human_recognition')
        self.parent.detection_human_enable_yes.setChecked(True) if self.ret['enabled'] else self.parent.detection_human_enable_no.setChecked(True)
        self.linkTextSlider(self.parent.detection_human_sensitivity, self.parent.detection_human_sensitivity_slider, int(self.ret['sensitivity']))
        for i, zone in enumerate(self.ret['path']):
            self.parent.scene.zones[i].update(zone)
        for i in range(4):
            self.parent.scene.zones[i]['enabled'] = True
        
        self.parent.scene.play_video(self.parent.detection_human_graphic_view)
        self.parent.scene.draw_rectangles(self.parent.detection_human_graphic_view)
        self.parent.scene.enableZoneEdit = True

    def motion_event(self):
        x,y,w,h = self.parent.scene.zones[0]['x'], self.parent.scene.zones[0]['y'], self.parent.scene.zones[0]['width'], self.parent.scene.zones[0]['height']
        x0 = self.parent.detection_motion_graphic_view.width() * x / 100
        y0 = self.parent.detection_motion_graphic_view.height() * y / 100
        x1 = x0 + self.parent.detection_motion_graphic_view.width() * w / 100
        y1 = y0 + self.parent.detection_motion_graphic_view.height() * h / 100
        # print(x0,y0,x1,y1)
        cols, rows = len(self.ret['rect_lists'][0]), len(self.ret['rect_lists'])
        box_width  = self.parent.detection_motion_graphic_view.width() / cols
        box_height = self.parent.detection_motion_graphic_view.height() / rows        
        
        coord = []
        for y in np.arange(y0, y1+box_height, box_height):
            for x in np.arange(x0, x1+box_width, box_width):
                coord.append((x,y))
        # print (coord)                        
        for j in range(rows):
            for i in range(cols):
                pos = self.rect_item[j][i].rect()
                xx0, yy0, xx1, yy1 = pos.x(), pos.y(), pos.x()+pos.width(), pos.y()+pos.height()
                if y0 > yy1 or y1 < yy0: 
                    break
                if x0 > xx1 or x1 < xx0:
                    continue

                for (x,y) in coord:
                    if  (xx0 <= x <= xx1 and yy0 <= y <= yy1) :
                        # print (j, i, x, y,pos)
                        self.rect_item[j][i].setVisible(False) if self.rect_item[j][i].isVisible() else self.rect_item[j][i].setVisible(True)
        # print()
    def view_detection_motion(self):
        self.ret = self.query('warning','motion_detection')
        self.parent.detection_motion_enable_yes.setChecked(True) if self.ret['enabled'] == True else self.parent.detection_motion_enable_no.setChecked(True)
        self.linkTextSlider(self.parent.detection_motion_sensitivity, self.parent.detection_motion_sensitivity_slider, int(self.ret['sensitivity']))

        cols, rows = len(self.ret['rect_lists'][0]), len(self.ret['rect_lists'])
        self.parent.detect_motion_ratio_enum.clear()
        for i, e in enumerate(self.ret['ratio_enum']):
            x = e // 100
            y = e - x * 100
            item = "%dx%d" %(x,y)
            self.parent.detect_motion_ratio_enum.addItem(item, e)
            if x == cols and y == rows:
                self.parent.detect_motion_ratio_enum.setCurrentIndex(i)      
        self.parent.detect_motion_ratio_enum.hide()
        box_width  = self.parent.detection_motion_graphic_view.width() / cols
        box_height = self.parent.detection_motion_graphic_view.height() / rows
        # print(box_width, box_height)
        self.rect_item = []
        for j in range(rows):
            r_item = []
            for i in range(cols):
                r = QGraphicsRectItem()
                self.parent.scene.addItem(r)
                r.setRect(i*box_width+2, j*box_height+2, box_width-2, box_height-2)
                r.setFlag(QGraphicsItem.ItemIsMovable, False)
                r.setBrush(QColor(127, 0, 0, 48))
                r.setPen(QColor(255, 255, 255, 1))
                r.setVisible(True)
                if not self.ret['rect_lists'][j][i]:
                    # r.hide()
                    r.setVisible(False)
                r_item.append(r)
            self.rect_item.append(r_item)
        self.parent.scene.play_video(self.parent.detection_motion_graphic_view)
        self.parent.scene.draw_rectangles(self.parent.detection_motion_graphic_view)
        
        self.parent.scene.enableZoneEdit = True
        self.parent.scene.current_zone = 0
        self.parent.scene.zones[self.parent.scene.current_zone]['enabled'] = True

    def view_detection_tamper(self):
        self.ret = self.query('warning', 'occlusion_detection')
        self.parent.detection_tamper_enable_yes.setChecked(True) if self.ret['enabled'] == True else self.parent.detection_tamper_enable_no.setChecked(True)
        self.linkTextSlider(self.parent.detection_tamper_sensitivity, self.parent.detection_tamper_sensitivity_slider, int(self.ret['sensitivity']))
        self.parent.scene.play_video(self.parent.detection_tamper_graphic_view)            
        self.parent.scene.enableZoneEdit = False

    def view_peripherial_alarm(self):
        pass
    def view_peripherial_serial(self):
        pass
    def view_peripherial_ptz(self):
        pass
    def view_peripherial_sd(self):
        pass

    def view_system_user(self):
        self.ret = self.query('system', 'user_management')
        print (self.ret)
        widget_list= [self.parent.system_user_info_widget, self.parent.btn_system_user_add, self.parent.btn_system_user_modify, self.parent.btn_system_user_delete]
        for w in widget_list:
            w.hide()

        def show_item_from_system_users(obj, e):
            self.parent.system_user_name.clear()
            self.parent.system_user_password.clear()
            self.parent.system_user_password2.clear()
            self.parent.system_user_group.setCurrentIndex(0)
            self.parent.system_user_no.setChecked(False)
            self.parent.system_user_yes.setChecked(False)
            self.parent.btn_system_user_add.show()
            self.parent.btn_system_user_modify.hide()
            self.parent.btn_system_user_delete.hide()
            self.parent.system_user_info_widget.show()
            self.parent.system_user_name.setReadOnly(False)

            id = obj.text(1)
            for uid in self.ret['users']:
                if id == uid['name']:
                    self.parent.system_user_name.setText(uid['name'])
                    self.parent.system_user_name.setReadOnly(True)
                    self.parent.system_user_group.setCurrentIndex(uid['classify'])
                    self.parent.system_user_yes.setChecked(True) if uid['enabled'] else self.parent.system_user_no.setChecked(True)
                    self.parent.btn_system_user_add.hide()
                    self.parent.btn_system_user_modify.show()
                    self.parent.btn_system_user_delete.show()
                    break

        groups = []
        for i in range(self.parent.system_user_group.count()):
            groups.append(self.parent.system_user_group.itemText(i))

        self.parent.user_tree.clear()
        for i,id in enumerate(self.ret['users']):
            item = QTreeWidgetItem(self.parent.user_tree)
            item.setText(0, str(i+1))
            item.setText(1, id['name'])
            item.setText(2, groups[id['classify']])
            item.setText(3, 'Yes') if id['enabled'] else item.setText(3, 'No')
            # s.user_tree.setItemWidget(item,4,QPushButton())
        item = QTreeWidgetItem(self.parent.user_tree)
        item.setText(0, "New")

        if self.connect_flags.get('system_user'): # add , reconnect
            self.parent.user_tree.itemDoubleClicked.disconnect()
        
        self.parent.user_tree.itemDoubleClicked.connect(show_item_from_system_users)
        self.connect_flags['system_user'] = True



    def view_system_datetime(self):
        self.ret = self.query('system','system_time')
        self.dev_ts = self.ret['manual_mode']['time']
        def update_current_datetime():
            self.parent.pc_datetime.setText(str(datetime.datetime.fromtimestamp(int(time.time()))))
            dt_str = datetime.datetime.fromtimestamp(self.dev_ts)
            self.parent.device_datetime.setText(str(dt_str))
            self.dev_ts +=1

        if not self.connect_flags.get('system_datetime'):
            self.onesec_timer.timeout.connect(update_current_datetime)
            # self.onesec_timer.start()
            self.connect_flags['system_datetime'] = True

        self.parent.system_datetime_date.setDate(QDate.currentDate())
        self.parent.system_datetime_time.setTime(QTime.currentTime())

        self.parent.radio_ntp_yes.setChecked(True) if self.ret['auto_mode'] == True else self.parent.radio_ntp_no.setChecked(True)
        self.parent.ntp_server.setText(self.ret['ntp_server']['name'])
        self.parent.ntp_port.setValue(self.ret['ntp_server']['port'])
        self.parent.sync_time.setValue(self.ret['ntp_server']['sync_time']) # spinbox ??

        self.parent.timezone.clear()
        x= 0
        for i in range(-12, 14):
            self.parent.timezone.addItem('GMT%d' %i if i<0 else 'GMT+%d' %i, i*100)
            if i*100 == self.ret['timezone']:
                self.parent.timezone.setCurrentIndex(x)
            x += 1

    def view_system_maintenance(self):
        self.ret = self.query('system', 'maintenance')
        self.parent.system_maintenance_yes.setChecked(True) if self.ret['enabled'] else self.parent.system_maintenance_no.setChecked(True)
        day_week = [self.parent.cksm_sun, self.parent.cksm_mon, self.parent.cksm_tue, self.parent.cksm_wed, self.parent.cksm_thu, self.parent.cksm_fri, self.parent.cksm_sat]
        for i, day in enumerate(day_week):
            day.setChecked(True) if (self.ret['day']>>i) & 1  else day.setChecked(False)    
  
        ts_f = self.ret['restart_time'].split(":")
        self.parent.system_restart_time.setTime(QTime(int(ts_f[0]),int(ts_f[1]),int(ts_f[2])))
        
    def view_system_about(self):
        self.ret = self.query('system', 'system_info')
        self.up_ts = self.ret['runtime']

        def update_uptime():
            hour =   self.up_ts // 3600
            minute = (self.up_ts%3600)  // 60
            second = self.up_ts % 60
            self.parent.system_about_device_uptime.setText("%02d:%02d:%02d" %(hour, minute, second))
            self.up_ts += 1

        if not self.connect_flags.get('system_about'):
            self.onesec_timer.timeout.connect(update_uptime)
            # self.onesec_timer.start()
            self.connect_flags['system_about'] = True

        self.parent.system_about_device_name.setText(self.ret['device_name'])
        self.parent.system_about_device_version.setText(self.ret['software_version'])
        self.parent.system_about_firmware_version.setText(self.ret['firmware_version'])

