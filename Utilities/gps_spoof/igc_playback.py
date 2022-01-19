#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import pygame
import serial
from pygame.locals import *

from datetime import datetime
import struct
import zlib

GNSS_MUL = 10000000
ZOOM_MUL = 100000

lat_mult = [
	1.00001269263410,
	1.00016503034732,
	1.00062227564598,
	1.00138512587137,
	1.00245474614468,
	1.00383277371586,
	1.00552132409524,
	1.00752299900955,
	1.00984089623881,
	1.01247862140355,
	1.01544030178698,
	1.01873060229028,
	1.02235474363937,
	1.02631852297526,
	1.03062833698348,
	1.03529120773669,
	1.04031481145009,
	1.04570751037551,
	1.05147838808628,
	1.05763728844289,
	1.06419485855898,
	1.07116259613477,
	1.07855290156473,
	1.08637913528301,
	1.09465568086611,
	1.10339801447867,
	1.11262278132588,
	1.12234787986046,
	1.13259255459192,
	1.14337749845791,
	1.15472496584871,
	1.16665889752463,
	1.17920505883836,
	1.19239119287568,
	1.20624719035817,
	1.22080527842164,
	1.23610023069989,
	1.25216960150919,
	1.26905398736196,
	1.28679731954458,
	1.30544719209465,
	1.32505523022013,
	1.34567750504505,
	1.36737500157145,
	1.39021414794172,
	1.41426741553005,
	1.43961400112117,
	1.46634060453179,
	1.49454231757694,
	1.52432364338688,
	1.55579966888401,
	1.58909741791146,
	1.62435741829468,
	1.66173552331931,
	1.70140503710806,
	1.74355920469978,
	1.78841414194345,
	1.83621229854950,
	1.88722657098226,
	1.94176521201939,
	2.00017772298141,
];


class GPS_Spoof(object):
    
    black = 0, 0, 0
    
    win_size = (1028, 768)
    done = False
    click = False
    last_point = 0
    
    points = []
    point_radius = 2
    point_color = 0, 100, 255
    point_color_used = 200, 200, 200
    point_timer = 0
    point_period = 1000
    point_index = 0
    
    mouse_point = None
    
    last_time = None   

    lat = 0
    lon = 0
    
    need_redraw = True
    
    zoom = 10
    
    fix = True
        
    skip = 0
    skip_timer = 0
    skip_period = 50
    
    pause = False
    
    def add_igc(self, filename):
        print("Loading IGC file", filename)
        f = open(filename, "r")
        lines = f.readlines()[1:]
        f.close()
        
        lat_min = 99
        lat_max = -lat_min
        
        lon_min = lat_min
        lon_max = -lat_min
        
        last_point = None
        
        for line in lines:
            f = line.replace("\n", "")
            if f[0] != 'B':
                continue
            #print(f)
            
            lat = f[7:15]
            lon = f[15:24]
            alt = f[25:30]
            
            lat_deg = int(lat[:2])
            lat_m = int(lat[2:][:-1]) / 60000.0

            lat = (lat_deg + lat_m) * (-1.0 if lat[-1] == 'S' else 1.0)
            
            lon_deg = int(lon[:3])
            lon_m = float(lon[3:][:-1]) / 60000.0
            
            lon = (lon_deg + lon_m) * (-1.0 if lon[-1] == 'W' else 1.0)

            alt = int(alt)
            
            if last_point:
                olatitude, olongitude, oalt, __, __, __ = last_point
    
                R = 6371e3 # metres
                phi_1 = math.radians(lat)
                phi_2 = math.radians(olatitude)
                delta_phi_ = math.radians(olatitude - lat);
                delta_lambda = math.radians(olongitude - lon);
                
                a = math.sin(delta_phi_/2) * math.sin(delta_phi_/2) + math.cos(phi_1) * math.cos(phi_2) * math.sin(delta_lambda/2) * math.sin(delta_lambda/2)
                c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
                
                d = R * c
                speed = d
    
                #hdg
                dx = lon - olongitude
                dy = lat - olatitude
                
                heading = math.degrees(math.atan2(dx, dy))
                if heading < 0:
                    heading += 360
                    
                vario = alt - oalt
            else:
                heading = 0
                speed = 0
                vario = 0            
            
            point = (lat, lon, alt, heading, speed, vario)
            
            last_point = point 

            self.points.append(point)

            lat_min = min(lat_min, lat)
            lat_max = max(lat_max, lat)
            lon_min = min(lon_min, lon)
            lon_max = max(lon_max, lon)
        
        self.lat_center = (lat_max + lat_min) / 2
        self.lon_center = (lon_max + lon_min) / 2
   
   
    def fake_igc(self, circles, samples, wind_x, wind_y, vario):
        
        lat_min = 99
        lat_max = -lat_min
        
        lon_min = lat_min
        lon_max = -lat_min
        
        last_point = None
        
        x = 0
        y = 0
        
        radius = 10
             
        self.lat_center = 0
        self.lon_center = 0          
        
        x_off = 0   
        y_off = 0
        a_off = 0
                
        for i in range(circles):
            for j in range(samples):
                angle = math.radians(j * 360 / samples);
                x = x_off - math.cos(angle) * radius;
                y = y_off - math.sin(angle) * radius;                      
                
                x_off += wind_x
                y_off += wind_y
                
                lat, lon = self.pix_to_geo(x, y)
                alt = a_off
                a_off += vario
            
                if last_point:
                    olatitude, olongitude, oalt, __, __, __ = last_point
        
                    R = 6371e3 # metres
                    phi_1 = math.radians(lat)
                    phi_2 = math.radians(olatitude)
                    delta_phi_ = math.radians(olatitude - lat);
                    delta_lambda = math.radians(olongitude - lon);
                    
                    a = math.sin(delta_phi_/2) * math.sin(delta_phi_/2) + math.cos(phi_1) * math.cos(phi_2) * math.sin(delta_lambda/2) * math.sin(delta_lambda/2)
                    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
                    
                    d = R * c
                    speed = d
        
                    #hdg
                    dx = lon - olongitude
                    dy = lat - olatitude
                    
                    heading = math.degrees(math.atan2(dx, dy))
                    if heading < 0:
                        heading += 360
                        
                else:
                    heading = 0
                    speed = 0
                
                point = (lat, lon, alt, heading, speed, vario)
                
                last_point = point 
    
                self.points.append(point)
    
                lat_min = min(lat_min, lat)
                lat_max = max(lat_max, lat)
                lon_min = min(lon_min, lon)
                lon_max = max(lon_max, lon)
        
        self.lat_center = (lat_max + lat_min) / 2
        self.lon_center = (lon_max + lon_min) / 2
   
   
    def main(self, port):
        pygame.init()
        self.screen = pygame.display.set_mode(self.win_size, RESIZABLE)
        self.clock = pygame.time.Clock()
        pygame.display.set_caption('Strato GNSS IGC playback')
        
        pygame.font.init()
        self.font = pygame.font.Font(pygame.font.get_default_font(), 20)
        
        self.altitiude = 200
        self.heading = 0
        self.speed = 0
        self.vario = 0
        
        try:
            self.port = serial.Serial(port, 921600)
        except:
            self.port = False
        
    def run(self):
        while not self.done:
            self.event()
            self.draw()
            self.clock.tick(30)

    def draw_text(self, text, x, y):
        tmp = self.font.render(text, True, (255, 10, 10))
        self.screen.blit(tmp,  [x, y])
    
    def draw_text_center(self, text, x, y):
        tmp = self.font.render(text, True, (255, 10, 10))
        w, h = tmp.get_size()
        self.screen.blit(tmp,  [int(x - w / 2), int(y - h / 2)])    
    
    def send_point(self, point):
        latitude, longitude, alt, hdg, spd, var = point
        
        time = int(datetime.utcnow().timestamp())

        self.lat = latitude
        self.lon = longitude
        self.altitiude  = alt
        self.heading = hdg
        self.speed = spd
        self.vario = var
        
        self.last_point = point
        
        if self.port is not False:
    
            data = struct.pack("<lllffff", int(time), int(latitude * GNSS_MUL), int(longitude * GNSS_MUL),
                                                    self.speed, self.heading, self.vario, self.altitiude)
    
            data += struct.pack("<L", zlib.crc32(data))
            self.port.write(b'f' + data)
        

    def event(self):
        
        for e in pygame.event.get():
            
            if e.type == QUIT or (e.type == KEYUP and e.key == K_ESCAPE):
                self.done = True
                break
                
            if e.type == KEYUP and e.key == ord('f'):
                self.fix = not self.fix
                self.need_redraw = True

            if e.type == KEYDOWN and e.key == K_LEFT:
                self.skip = -10
                if pygame.key.get_mods() & KMOD_SHIFT:
                    self.skip *= 10
                self.skip_timer = 0

            if e.type == KEYDOWN and e.key == K_RIGHT:
                self.skip = 10
                if pygame.key.get_mods() & KMOD_SHIFT:
                    self.skip *= 10
                self.skip_timer = 0

            if e.type == KEYDOWN and e.key == K_SPACE:
                self.pause = not self.pause
                self.need_redraw = True
                
            if e.type == KEYUP:
                self.skip = 0
                self.skip_timer = 0

            elif e.type == MOUSEBUTTONDOWN and e.button == 1:
                self.click = True
                self.need_redraw = True
                
                if not self.fix:
                    self.lat_center, self.lon_center = self.pix_to_geo(e.pos[0], e.pos[1])

            elif e.type == MOUSEBUTTONUP and e.button == 1:
                self.click = False
                self.need_redraw = True

            elif e.type == MOUSEBUTTONDOWN and e.button == 5:
                if self.zoom <= 140:
                    self.zoom += 1
                    
                self.need_redraw = True

            elif e.type == MOUSEBUTTONDOWN and e.button == 4:
                if self.zoom > 1:
                    self.zoom -= 1
                self.need_redraw = True

            elif e.type == MOUSEMOTION and self.click:
                self.need_redraw = True

            elif e.type == MOUSEMOTION:
                self.mouse_point = e.pos
                self.need_redraw = True
                
            elif e.type == pygame.VIDEORESIZE:
                self.screen = pygame.display.set_mode((e.w, e.h), RESIZABLE)
                self.win_size = (e.w, e.h)
                self.need_redraw = True
                
            else:
                pass
#                print(e)
                
            
          
        if len(self.points) > 0 and pygame.time.get_ticks() > self.point_timer and not self.pause:
            self.point_timer = pygame.time.get_ticks() + self.point_period
            self.send_point(self.points[self.point_index])
            
            self.point_index += 1
            
            self.need_redraw = True
            
        if self.skip != 0 and pygame.time.get_ticks() > self.skip_timer:
            self.skip_timer = pygame.time.get_ticks() + self.skip_period
            
            self.point_index += self.skip
            self.need_redraw = True

        self.point_index = max(1, self.point_index)            
        if (self.point_index >= len(self.points)):
            self.point_index = len(self.points) - 1
            
            
            
    def geo_to_pix(self, lat, lon):
        step_x = self.zoom / ZOOM_MUL
        lat_i = int(min(61, abs(self.lat_center)))
        step_y = self.zoom / lat_mult[lat_i] / ZOOM_MUL 

        map_w = self.win_size[0] * step_x
        map_h = self.win_size[1] * step_y
        lon1 = self.lon_center - map_w / 2
        lat1 = self.lat_center + map_h / 2

        d_lat = lat1 - lat
        d_lon = lon - lon1

        x =	int(d_lon / step_x)
        y = int(d_lat / step_y)

        return x, y 
        
    def pix_to_geo(self, x, y):
        step_x = self.zoom  / ZOOM_MUL
        lat_i = int(min(61, abs(self.lat_center)))
        step_y = self.zoom / lat_mult[lat_i]  / ZOOM_MUL

        map_w = self.win_size[0] * step_x
        map_h = self.win_size[1] * step_y
        lon1 = self.lon_center - map_w / 2
        lat1 = self.lat_center + map_h / 2

        d_lat = y * step_y
        d_lon = x * step_x

        lon = d_lon + lon1
        lat = lat1 - d_lat 

        return lat, lon 
                
                    
    def draw(self):
        if not self.need_redraw:
            return
        self.need_redraw = False
        
        self.screen.fill(self.black)

        if self.fix:
            lat, lon = self.points[self.point_index][0:2]
            self.lat_center = lat
            self.lon_center = lon            
        
        last_point = None

        p = self.geo_to_pix(self.lat_center, self.lon_center)  
        pygame.draw.circle(self.screen, (255,0,0), p, self.point_radius)


        for index in range(len(self.points)):
            lat, lon = self.points[index][0:2]
            color = self.point_color if index > self.point_index else self.point_color_used
            
            point = self.geo_to_pix(lat, lon)

            pygame.draw.circle(self.screen, color, point, self.point_radius)
            if last_point:
                pygame.draw.line(self.screen, color, last_point, point)

            last_point = point

        self.draw_text("Alt: %d" % self.altitiude, 0, 0)
        self.draw_text("Hdg: %d" % self.heading, 0, 20)
        self.draw_text("Spd: %0.1f" % (self.speed * 1.852), 0, 40)
        self.draw_text("Var: %0.1f" % (self.vario), 0, 60)
            
        self.draw_text("Lat: %0.7f" % self.lat, 0, 90)
        self.draw_text("Lon: %0.7f" % self.lon, 0, 110)

        if self.mouse_point != None:
            ox, oy = self.mouse_point
            self.draw_text("x: %u" % ox, 0, 140)
            self.draw_text("y: %u" % oy, 0, 160)
            
            clat, clon = self.pix_to_geo(ox, oy)
            self.draw_text("cLat: %0.7f" % clat, 0, 200)
            self.draw_text("cLon: %0.7f" % clon, 0, 220)
            
        self.draw_text("Zoom: %0.1f" % self.zoom, 0, 180)
        if self.pause:
            self.draw_text("PAUSED", 0, 240)
        
        
        self.calc_wind()
        
        
        pygame.display.update()     
        
    def calc_wind(self):
        index = self.point_index

        sector_cnt = 16

        hdg_acc = [0] * sector_cnt
        spd_acc = [0] * sector_cnt
        cnt = [0] * sector_cnt
        
        max_samples = 100
        
        while index >= 0 and max_samples > 0:
            lat, lon, __, hdg, spd, __ = self.points[index]
            
            point = self.geo_to_pix(lat, lon)

            pygame.draw.circle(self.screen, (0,255,0), point, self.point_radius)            
            
            sector = int(math.floor(hdg / (360 / sector_cnt)))
            hdg_acc[sector] += hdg
            spd_acc[sector] += spd
            cnt[sector] += 1
            
            max_samples -= 1
            index -= 1

                    
            
        cx = 250
        cy = 500
        rad = 200
            
        hdg = [0] * sector_cnt
        spd = [0] * sector_cnt
        diff = [0] * sector_cnt
        
        #only for visuals
        max_val = -1
        
        used_cnt = 0
        
        for i in range(sector_cnt):
            if cnt[i] > 0: 
                used_cnt += 1
                
                hdg[i] = hdg_acc[i] / cnt[i]
                spd[i] = spd_acc[i] / cnt[i]        
                
                if spd[i] > max_val:
                    max_val = spd[i]

                
        for i in range(sector_cnt):
            if cnt[i] > 0:                 
                diff[i] = spd[(i + int(sector_cnt / 2)) % sector_cnt] - spd[i]
                 
            #visuals   
            angle = math.radians(90 + (i + 0.5) * (360 / sector_cnt));
            x = int(-math.cos(angle) * rad);
            y = int(-math.sin(angle) * rad);
            
            self.draw_text_center("[%u]" % i, cx + x, cy + y - 20)             
            self.draw_text_center("%0.1f" % (hdg[i]), cx + x, cy + y)             
            self.draw_text_center("%0.1f (%u)" % (spd[i], cnt[i]), cx + x, cy + y + 20)
            self.draw_text_center("d%0.1f" % (diff[0]), cx + x, cy + y + 40)
        
        #visuals
        if max_val > 0:
            for i in range(sector_cnt):
                angle = math.radians(90 + (i + 0.5) * (360 / sector_cnt));
                x = int(-math.cos(angle) * rad * 3 / 5);
                y = int(-math.sin(angle) * rad * 3 / 5);
                         
                pygame.draw.circle(self.screen, (255,0,255), (cx + x, cy + y), int((spd[i] / max_val) * 15))
        
        
        wind_hdg = 0
        wind_spd = 0

        #visuals
        wind_gain = 100 / 6.0
        
        if sector_cnt == used_cnt:
            x_acc = 0
            y_acc = 0
            for i in range(sector_cnt):
                angle = math.radians(90 + 180 + hdg[i]);
                
                x = math.cos(angle) * diff[i]
                y = math.sin(angle) * diff[i]
                
                x_acc += x
                y_acc += y
                
                #visuals
                tx = int(cx - x * wind_gain);
                ty = int(cy - y * wind_gain);       
                
                color = (0, 0, 255) if diff[i] > 0 else (255, 255, 0)
                pygame.draw.line(self.screen, color, (cx, cy), (tx, ty))          
                pygame.draw.circle(self.screen, color, (tx, ty), 5)          
    
            wind_x = x_acc / sector_cnt
            wind_y = y_acc / sector_cnt
            
            tx = int(cx - wind_x * wind_gain)
            ty = int(cy - wind_y * wind_gain)
            
            pygame.draw.line(self.screen, (0, 255, 255), (cx, cy), (tx, ty), 3)
            
            wind_hdg = int(math.degrees(math.atan2(wind_x, -wind_y)) + 360) % 360
            wind_spd = math.sqrt(pow(wind_x, 2) + pow(wind_y, 2))
    
        #visuals
        ndl = wind_spd * wind_gain
        angle = math.radians(90 + 180 + wind_hdg);
        tx = int(cx - math.cos(angle) * ndl);
        ty = int(cy - math.sin(angle) * ndl);       
        
        pygame.draw.line(self.screen, (0, 255, 0), (cx, cy), (tx, ty))

        angle = math.radians(wind_hdg);
        sx1 = int(cx - math.cos(angle) * ndl / 8);
        sy1 = int(cy - math.sin(angle) * ndl / 8);       

        angle = math.radians(wind_hdg + 180);
        sx2 = int(cx - math.cos(angle) * ndl / 8);
        sy2 = int(cy - math.sin(angle) * ndl / 8);       

        pygame.draw.line(self.screen, (0, 255, 0), (sx1, sy1), (tx, ty))
        pygame.draw.line(self.screen, (0, 255, 0), (sx2, sy2), (tx, ty))
        pygame.draw.line(self.screen, (0, 255, 0), (sx1, sy1), (sx2, sy2))

        if used_cnt == sector_cnt:
            self.draw_text_center("%0.1f" % (wind_hdg), cx, cy + 20)
            self.draw_text_center("%0.1f m/s" % (wind_spd), cx, cy + 40)
        else:
            self.draw_text_center("Not valid", cx, cy + 20)
        
if __name__ == '__main__':
    o = GPS_Spoof()

    o.main("/dev/ttyACM0")
    o.add_igc("wind.igc")
    
    #o.fake_igc(5, 40, 0.1, 0.1, 1)
    
    o.run()
    
    
    
