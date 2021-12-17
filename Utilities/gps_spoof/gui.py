#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import pygame
import serial
from pygame.locals import *

from datetime import datetime
from time import time
import struct
import zlib
import random

GPS_MUL = 10000000

class GPS_Spoof(object):
    
    black = 0, 0, 0
    
    win_size = (800, 600)
    done = False
    click = False
    last_point = 0
    
    points = []
    point_radius = 2
    point_color = 0, 100, 255
    point_color_used = 200, 200, 200
    point_timer = 0
    point_period = 1000
    #point_min_dist = 15
    point_min_time = 0.05
    point_index = 0
    
    mouse_point = None
    
    last_point = None
    last_time = None   
    
    lat_start = 48.1485965
    lon_start = 17.1077477
    
    lat = 0
    lon = 0
    
    gain = 0.000005

    waypoints = []
    wpt_color = 0, 255, 0
    wpt_radius = 4
    
    thermals = []
    thermal_color = 100, 0, 0
    
    need_redraw = True
    rnd = False
    rnd_timer = 0
    
    def add_waypoints(self, filename):
        print("Loading file", filename)
        f = open(filename, "r")
        lines = f.readlines()[1:]
        f.close()
        
        lat_min = 999999999999999
        lat_max = -lat_min
        
        lon_min = lat_min
        lon_max = -lat_min
        
        for line in lines:
            f = line.replace("\n", "").split(",")
            #print(f)
            
            if f[0] == "-----Related Tasks-----":
                break
            
            name = f[1].replace("\"", "")
            lat = f[3]
            lon = f[4]
            
            lat_deg = int(lat[:2])
            lat_m = float(lat[2:][:-1]) / 60.0
            
            lat = (lat_deg + lat_m) * (-1.0 if lat[-1] == 'S' else 1.0)
            
            lon_deg = int(lon[:3])
            lon_m = float(lon[3:][:-1]) / 60.0
            
            lon = (lon_deg + lon_m) * (-1.0 if lon[-1] == 'W' else 1.0)

            print(name, lat, lon)
            
            lat_min = min(lat_min, lat)
            lat_max = max(lat_max, lat)
            lon_min = min(lon_min, lon)
            lon_max = max(lon_max, lon)
            
            self.waypoints.append([lat, lon, name])
        
        
        border = 0.1
        lat_min -= border
        lat_max += border
        lon_min -= border
        lon_max += border
        
        self.lat_start = lat_min
        self.lon_start = lon_min
        
        delta = max(lat_max - lat_min, lon_max - lon_min)
        self.gain = delta / max(self.win_size)
        
        print(len(self.waypoints), "points loaded")
    
   
    def main(self, port):
        pygame.init()
        self.screen = pygame.display.set_mode(self.win_size, RESIZABLE)
        self.clock = pygame.time.Clock()
        pygame.display.set_caption('Strato GPS simulator')
        
        pygame.font.init()
        self.font = pygame.font.Font(pygame.font.get_default_font(), 20)
        
        self.altitiude = 200
        self.heading = 0
        self.speed = 0
        self.vario = 0
        
        self.port = serial.Serial(port, 921600)
        
    def run(self):
        while not self.done:
            self.event()
            self.draw()
            self.clock.tick(30)

    def draw_text(self, text, x, y):
        tmp = self.font.render(text, True, (255, 10, 10))
        self.screen.blit(tmp,  [x, y])
    
                    
    def add_point(self, pos, alt):
#         if len(self.points):
#             last_point = self.points[-1]
#             dist = math.sqrt((last_point[0] - pos[0]) ** 2 + (last_point[1] - pos[1]) ** 2)
#             if dist < self.point_min_dist:
#                 return

        if self.last_time:
            if time() - self.last_time < self.point_min_time:
                return

        self.last_time = time()
        
        a = pos[0], pos[1], alt
        
        self.points.append(a)
        
    def coord_to_point(self, lat, lon, __):
        y = int((lat - self.lat_start) / self.gain)
        x = int((lon - self.lon_start) / self.gain)       

        return x, self.win_size[1] - y
            
                    
    def send_point(self, point):
        x, y, alt = point
        
        time = int(datetime.utcnow().timestamp())

        latitude = self.lat_start + (self.win_size[1] - y) * self.gain
        longitude = self.lon_start + x * self.gain
        
        self.lat = latitude
        self.lon = longitude
       
        if self.last_point:
            ox, oy = self.last_point[0:2]

            olatitude = self.lat_start + (self.win_size[1] - oy) * self.gain
            olongitude = self.lon_start + ox * self.gain 
                    
            R = 6371e3 # metres
            phi_1 = math.radians(latitude)
            phi_2 = math.radians(olatitude)
            delta_phi_ = math.radians(olatitude - latitude);
            delta_lambda = math.radians(olongitude - longitude);
            
            a = math.sin(delta_phi_/2) * math.sin(delta_phi_/2) + math.cos(phi_1) * math.cos(phi_2) * math.sin(delta_lambda/2) * math.sin(delta_lambda/2)
            c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
            
            d = R * c
            self.speed = d

            #hdg
            dx = x - ox
            dy = oy - y
            
            self.heading = math.degrees(math.atan2(dx, dy))
            if self.heading < 0:
                self.heading += 360
        else:
            self.heading = 0
            self.speed = 0
        
        self.last_point = point
        
        #vario
        self.vario = -1
        for t in self.thermals:
            tx, ty, tr, ts = t
            
            d = math.sqrt(math.pow(tx - x, 2) + math.pow(ty - y, 2))
            if d > tr:
                continue
                
            self.vario += 1 + ts * (1 - d / tr)
        self.altitiude += self.vario
            
        
        data = struct.pack("<lllffff", int(time), int(latitude * GPS_MUL), int(longitude * GPS_MUL),
                                                self.speed, self.heading, self.vario, self.altitiude)

        data += struct.pack("<L", zlib.crc32(data))
        #for c in data:
        #    print("%02X " % c, end="")
        #print()
        self.port.write(b'f' + data)
        

    def event(self):
        if self.rnd and self.rnd_timer < time():
            self.rnd_timer = time() + 1
            x = int(random.random() * self.win_size[0])
            y = int(random.random() * self.win_size[1])
            
            self.add_point((x, y), self.altitiude)
            self.need_redraw = True
    
        for e in pygame.event.get():
            
            if e.type == QUIT or (e.type == KEYUP and e.key == K_ESCAPE):
                self.done = True
                break
                
            if e.type == KEYUP and e.key == ord('r'):
                self.rnd = not self.rnd
                self.need_redraw = True

            elif e.type == MOUSEBUTTONDOWN and e.button == 1:
                self.add_point(e.pos, self.altitiude)
                self.click = True
                self.need_redraw = True
                
            elif e.type == MOUSEBUTTONDOWN and e.button == 3:
                self.points = []
                self.point_timer = 0
                self.point_index = 0   
                self.need_redraw = True             

            elif e.type == MOUSEBUTTONDOWN and e.button == 4:
                self.altitiude += 10
                self.need_redraw = True

            elif e.type == MOUSEBUTTONDOWN and e.button == 5:
                self.altitiude -= 10
                self.need_redraw = True

            elif e.type == MOUSEBUTTONUP and e.button == 1:
                self.click = False
                
            elif e.type == MOUSEMOTION and self.click:
                self.add_point(e.pos, self.altitiude)
                self.need_redraw = True

            elif e.type == MOUSEMOTION:
                self.mouse_point = e.pos
                self.need_redraw = True
                
            elif e.type == pygame.VIDEORESIZE:
                self.screen = pygame.display.set_mode((e.w, e.h), RESIZABLE)
                self.win_size = (e.w, e.h)
                self.need_redraw = True
                
            
          
        if len(self.points) > 0 and pygame.time.get_ticks() > self.point_timer:
            self.point_timer = pygame.time.get_ticks() + self.point_period
            self.send_point(self.points[self.point_index])
            
            self.point_index += 1
            if (self.point_index >= len(self.points)):
                self.point_index = len(self.points) - 1 
            
            self.need_redraw = True
                    
    def draw(self):
        if not self.need_redraw:
            return
        self.need_redraw = False
        
        self.screen.fill(self.black)
        
        last_point = None


        for t in self.thermals:
            tx, ty, tr, ts = t
            pygame.draw.circle(self.screen, self.thermal_color, (tx, ty), tr)
            self.draw_text("%0.1f m/s" % ts, tx, ty)


        for index in range(len(self.points)):
            point = self.points[index][0:2]
            color = self.point_color if index > self.point_index else self.point_color_used
            
            pygame.draw.circle(self.screen, color, point, self.point_radius)
            if last_point:
                pygame.draw.line(self.screen, color, last_point, point)
            last_point = point
            
        for wpt in self.waypoints:
            point = self.coord_to_point(*wpt)
            pygame.draw.circle(self.screen, self.wpt_color, point, self.wpt_radius)
            self.draw_text(wpt[2], point[0] - 40, point[1] + 8)

            
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

        if self.rnd:
            self.draw_text("rnd active", 0, 180)
        
        pygame.display.update()     
        
        
if __name__ == '__main__':
    o = GPS_Spoof()

    o.main("/dev/ttyACM0")
    #o.add_waypoints("route.cup")
    o.thermals = [[400,200, 150, 3]]
    
    o.run()
    
    
    
