#! /usr/bin/python
from __future__ import division, absolute_import, print_function, unicode_literals


import sys as system
import os as os
import serial as serial
import shutil as shutil
import time as time
import struct as struct
import ctypes
import datetime
import Queue
import wx
from collections import namedtuple
import argparse

import logging

# The recommended way to use wx with mpl is with the WXAgg
# backend. 
#
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
	FigureCanvasWxAgg as FigCanvas, \
	NavigationToolbar2WxAgg as NavigationToolbar
import numpy as np
import pylab


FILE_NAME_PREFIX = "logging_"
FILE_EXTENSION = ".csv"
SYNC_WORD = 'CE'
datestr = None

serial_port = None
SerialData = namedtuple('SerialData', 'timestamp, mac rss data battery')


dataQueue = Queue.Queue()

get_input = input if system.version_info[0] >= 3 else raw_input # TODO compatibility between python 2 and 3, can be removed if we switch to python 3 (waiting on matplotlib)

logging.Formatter(fmt='%(asctime)s.%(msecs)d',datefmt='%Y-%m-%d,%H:%M:%S')


def read_value_from_serial():
	data = serial_port.read()
	#print(data.encode("hex").upper());
	while not data.encode("hex").upper() == SYNC_WORD:
		print("received unexpected data (%s), waiting for sync word " % data.encode("hex").upper())
		#print(data.encode("hex").upper());
		data = serial_port.read()
	
	data = serial_port.read()
	#print(data.encode("hex").upper());	
	length = struct.unpack("b", data)[0]  - 11

	#print("length: %d" % length);
	data = serial_port.read()
	#print(data.encode("hex").upper());	
	
	if data == '\x10':
		mac = serial_port.read(size=8).encode("hex").upper()
		#print(mac)
	else:
		return None;
	
	data = serial_port.read()
	#print(data.encode("hex").upper());	
	
	if data == '\x20':
		rss = struct.unpack("b", serial_port.read(size=1))[0]
		#print(rssi)
	else:
		return None;
		
	data = serial_port.read(size=length)
	payload = data.encode("hex").upper()
	
	counter = 0
	battery = 0
	battery2 = 0
	lqi = 0
	lqi2 = 0
	mac2 = None
	rss2 = 0
	inpayload = 0
	payload2 = None
	while counter < length:
		# print(data[counter].encode("hex").upper())
		code = struct.unpack('B', data[counter])[0]
		if code == 129: #0x81
			mac2 = data[counter+1:counter+9].encode("hex").upper() + "-" + mac
			# print(data[counter+9].encode("hex").upper())
			rss2 = struct.unpack("b", data[counter+9])[0]
			counter += 9
			
		elif code == 101: #0x65
			if inpayload:				
				battery2 = (256 * int(struct.unpack('B', data[counter+1])[0]) + int(struct.unpack('B', data[counter+2])[0])) / 100.0
			else:
				battery = (256 * int(struct.unpack('B', data[counter+1])[0]) + int(struct.unpack('B', data[counter+2])[0])) / 100.0
			counter += 2
			
		elif code == 130: #0x82		
			if inpayload:
				lqi2 = struct.unpack("b", data[counter+1])[0]
			else:
				lqi = struct.unpack("b", data[counter+1])[0]
			counter += 1		
			
		elif code == 153: #0x99
			dialogid2 = struct.unpack("b", data[counter+1])[0]
			counter += 1
			
		elif code == 240: #0xf0
			inpayload = 1
			payload2 = data[counter+1:].encode("hex").upper()
			counter += 1
			
		counter += 1
		
	data = serial_port.read()
	#print(data.encode("hex").upper());
	
	if data != '\x0D':
		print("received unexpected data (%s), expecting tail 0x0d" % data.encode("hex").upper())
		return None;

		
	serialData = SerialData(datetime.datetime.now(), mac, rss, payload, battery)
	if serialData != None:
		dataQueue.put(serialData)
		
	if mac2 != None:
		serialData = SerialData(datetime.datetime.now(), mac2, rss2, payload2, battery2)
		if serialData != None:
			dataQueue.put(serialData)

def empty_serial_buffer():
	while serial_port.inWaiting() > 0:
		serial_port.read(1)

class BoundControlBox(wx.Panel):
	""" A static box with a couple of radio buttons and a text
		box. Allows to switch between an automatic mode and a 
		manual mode with an associated value.
	"""
	def __init__(self, parent, ID, label, initval):
		wx.Panel.__init__(self, parent, ID)
		
		self.value = initval
		
		box = wx.StaticBox(self, -1, label)
		sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
		
		self.radio_auto = wx.RadioButton(self, -1, 
			label="Auto", style=wx.RB_GROUP)
		self.radio_manual = wx.RadioButton(self, -1,
			label="Manual")
		self.manual_text = wx.TextCtrl(self, -1, 
			size=(35,-1),
			value=str(initval),
			style=wx.TE_PROCESS_ENTER)
		
		self.Bind(wx.EVT_UPDATE_UI, self.on_update_manual_text, self.manual_text)
		self.Bind(wx.EVT_TEXT_ENTER, self.on_text_enter, self.manual_text)
		
		manual_box = wx.BoxSizer(wx.HORIZONTAL)
		manual_box.Add(self.radio_manual, flag=wx.ALIGN_CENTER_VERTICAL)
		manual_box.Add(self.manual_text, flag=wx.ALIGN_CENTER_VERTICAL)
		
		sizer.Add(self.radio_auto, 0, wx.ALL, 10)
		sizer.Add(manual_box, 0, wx.ALL, 10)
		
		self.SetSizer(sizer)
		sizer.Fit(self)
	
	def on_update_manual_text(self, event):
		self.manual_text.Enable(self.radio_manual.GetValue())
	
	def on_text_enter(self, event):
		self.value = self.manual_text.GetValue()
	
	def is_auto(self):
		return self.radio_auto.GetValue()
		
	def manual_value(self):
		return self.value
		
		
class GraphFrame(wx.Frame):
	""" The main frame of the application
	"""
	title = 'DASH7 Demo plot RSS of star network'
	
	def __init__(self):
		wx.Frame.__init__(self, None, -1, self.title)
		
		#self.datagen = DataGen()
		self.data = [0]
		self.paused = False
		
		self.create_menu()
		self.create_status_bar()
		self.create_main_panel()
		
		self.redraw_timer = wx.Timer(self)
		self.Bind(wx.EVT_TIMER, self.on_redraw_timer, self.redraw_timer)		
		self.redraw_timer.Start(100)
		
		global serial_port, settings
		
		parser = argparse.ArgumentParser(description = "DASH7 logger for the OSS-7 stack. You can exit the logger using Ctrl-c, it takes some time.")
		parser.add_argument('serial', default="COM4", metavar="serial port", help="serial port (eg COM7 or /dev/ttyUSB0)", nargs='?')
		parser.add_argument('-b', '--baud' , default=9600, metavar="baudrate", type=int, help="set the baud rate (default: 9600)")
		settings = vars(parser.parse_args())
		
		serial_port = serial.Serial(settings['serial'], settings['baud'])
		datestr = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
		
		self.f = open(datestr + FILE_EXTENSION, 'w')
		self.f.write("timestamp, mac, rss, battery\n")
		self.f.flush()
		empty_serial_buffer()	

	def create_menu(self):
		self.menubar = wx.MenuBar()
		
		menu_file = wx.Menu()
		m_expt = menu_file.Append(-1, "&Save plot\tCtrl-S", "Save plot to file")
		self.Bind(wx.EVT_MENU, self.on_save_plot, m_expt)
		menu_file.AppendSeparator()
		m_exit = menu_file.Append(-1, "E&xit\tCtrl-X", "Exit")
		self.Bind(wx.EVT_MENU, self.on_exit, m_exit)
				
		self.menubar.Append(menu_file, "&File")
		self.SetMenuBar(self.menubar)

	def create_main_panel(self):
		self.panel = wx.Panel(self)

		self.init_plot()
		self.canvas = FigCanvas(self.panel, -1, self.fig)

		self.xmin_control = BoundControlBox(self.panel, -1, "X min", 0)
		self.xmax_control = BoundControlBox(self.panel, -1, "X max", 50)
		self.ymin_control = BoundControlBox(self.panel, -1, "Y min", 0)
		self.ymax_control = BoundControlBox(self.panel, -1, "Y max", 100)
		
		self.pause_button = wx.Button(self.panel, -1, "Pause")
		self.Bind(wx.EVT_BUTTON, self.on_pause_button, self.pause_button)
		self.Bind(wx.EVT_UPDATE_UI, self.on_update_pause_button, self.pause_button)
		
		self.cb_grid = wx.CheckBox(self.panel, -1, 
			"Show Grid",
			style=wx.ALIGN_RIGHT)
		self.Bind(wx.EVT_CHECKBOX, self.on_cb_grid, self.cb_grid)
		self.cb_grid.SetValue(True)
		
		self.cb_xlab = wx.CheckBox(self.panel, -1, 
			"Show X labels",
			style=wx.ALIGN_RIGHT)
		self.Bind(wx.EVT_CHECKBOX, self.on_cb_xlab, self.cb_xlab)		
		self.cb_xlab.SetValue(True)
		
		self.hbox1 = wx.BoxSizer(wx.HORIZONTAL)
		self.hbox1.Add(self.pause_button, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
		self.hbox1.AddSpacer(20)
		self.hbox1.Add(self.cb_grid, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
		self.hbox1.AddSpacer(10)
		self.hbox1.Add(self.cb_xlab, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
		
		self.hbox2 = wx.BoxSizer(wx.HORIZONTAL)
		self.hbox2.Add(self.xmin_control, border=5, flag=wx.ALL)
		self.hbox2.Add(self.xmax_control, border=5, flag=wx.ALL)
		self.hbox2.AddSpacer(24)
		self.hbox2.Add(self.ymin_control, border=5, flag=wx.ALL)
		self.hbox2.Add(self.ymax_control, border=5, flag=wx.ALL)
		
		self.vbox = wx.BoxSizer(wx.VERTICAL)
		self.vbox.Add(self.canvas, 1, flag=wx.LEFT | wx.TOP | wx.GROW)		
		self.vbox.Add(self.hbox1, 0, flag=wx.ALIGN_LEFT | wx.TOP)
		self.vbox.Add(self.hbox2, 0, flag=wx.ALIGN_LEFT | wx.TOP)
		
		self.panel.SetSizer(self.vbox)
		self.vbox.Fit(self)
	
	def create_status_bar(self):
		self.statusbar = self.CreateStatusBar()

	def init_plot(self):
		self.dpi = 100
		self.fig = Figure((3.0, 3.0), dpi=self.dpi)

		self.axes = self.fig.add_subplot(111)
		self.axes.set_axis_bgcolor('black')
		self.axes.set_title('Dash7 RSS', size=12)
		
		pylab.setp(self.axes.get_xticklabels(), fontsize=8)
		pylab.setp(self.axes.get_yticklabels(), fontsize=8)

		# plot the data as a line series, and save the reference 
		# to the plotted line series
		#
		self.plot_data = self.axes.plot(
			self.data, 
			linewidth=1,
			color=(1, 1, 0),
			)[0]

	def draw_plot(self):
		""" Redraws the plot
		"""
		# when xmin is on auto, it "follows" xmax to produce a 
		# sliding window effect. therefore, xmin is assigned after
		# xmax.
		#
		if self.xmax_control.is_auto():
			xmax = len(self.data) if len(self.data) > 50 else 50
		else:
			xmax = int(self.xmax_control.manual_value())
			
		if self.xmin_control.is_auto():			
			xmin = xmax - 50
		else:
			xmin = int(self.xmin_control.manual_value())

		# for ymin and ymax, find the minimal and maximal values
		# in the data set and add a mininal margin.
		# 
		# note that it's easy to change this scheme to the 
		# minimal/maximal value in the current display, and not
		# the whole data set.
		# 
		if self.ymin_control.is_auto():
			ymin = round(min(self.data), 0) - 1
		else:
			ymin = int(self.ymin_control.manual_value())
		
		if self.ymax_control.is_auto():
			ymax = round(max(self.data), 0) + 1
		else:
			ymax = int(self.ymax_control.manual_value())

		self.axes.set_xbound(lower=xmin, upper=xmax)
		self.axes.set_ybound(lower=ymin, upper=ymax)
		
		# anecdote: axes.grid assumes b=True if any other flag is
		# given even if b is set to False.
		# so just passing the flag into the first statement won't
		# work.
		#
		if self.cb_grid.IsChecked():
			self.axes.grid(True, color='gray')
		else:
			self.axes.grid(False)

		# Using setp here is convenient, because get_xticklabels
		# returns a list over which one needs to explicitly 
		# iterate, and setp already handles this.
		#  
		pylab.setp(self.axes.get_xticklabels(), 
			visible=self.cb_xlab.IsChecked())
		
		self.plot_data.set_xdata(np.arange(len(self.data)))
		self.plot_data.set_ydata(np.array(self.data))
		
		self.canvas.draw()
	
	def on_pause_button(self, event):
		self.paused = not self.paused
	
	def on_update_pause_button(self, event):
		label = "Resume" if self.paused else "Pause"
		self.pause_button.SetLabel(label)
	
	def on_cb_grid(self, event):
		self.draw_plot()
	
	def on_cb_xlab(self, event):
		self.draw_plot()
	
	def on_save_plot(self, event):
		file_choices = "PNG (*.png)|*.png"
		
		dlg = wx.FileDialog(
			self, 
			message="Save plot as...",
			defaultDir=os.getcwd(),
			defaultFile="plot.png",
			wildcard=file_choices,
			style=wx.SAVE)
		
		if dlg.ShowModal() == wx.ID_OK:
			path = dlg.GetPath()
			self.canvas.print_figure(path, dpi=self.dpi)
			self.flash_status_message("Saved to %s" % path)
	
	def on_redraw_timer(self, event):
		# if paused do not add data, but still redraw the plot
		# (to respond to scale modifications, grid change, etc.)
		#
		if not self.paused:
			read_value_from_serial()
			while not dataQueue.empty():
				serialData = dataQueue.get()
				self.data.append(serialData.rss)
				self.f.write("%s,%s,%s,%s,%s\n" %(serialData.timestamp.strftime("%Y-%m-%d %H:%M:%S.%f"), serialData.mac, serialData.rss, serialData.data, serialData.battery))
				print("%s,%s,%s,%s,%s" %(serialData.timestamp.strftime("%Y-%m-%d %H:%M:%S.%f"), serialData.mac, serialData.rss, serialData.data, serialData.battery))
				
		
		self.draw_plot()
	
	def on_exit(self, event):
		self.Destroy()
	
	def flash_status_message(self, msg, flash_len_ms=1500):
		self.statusbar.SetStatusText(msg)
		self.timeroff = wx.Timer(self)
		self.Bind(
			wx.EVT_TIMER, 
			self.on_flash_status_off, 
			self.timeroff)
		self.timeroff.Start(flash_len_ms, oneShot=True)
	
	def on_flash_status_off(self, event):
		self.statusbar.SetStatusText('')

			
if __name__ == '__main__':


	app = wx.PySimpleApp()
	app.frame = GraphFrame()
	app.frame.Show()
	app.MainLoop()
