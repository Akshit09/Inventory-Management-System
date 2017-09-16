import serial
import syslog
import time
import io
from openpyxl import load_workbook

port = '/dev/ttyACM0'

log = load_workbook('DataLog.xlsx')
sheet = log.active

ard = serial.Serial(port, 115200, timeout = 10)
time.sleep(2)

ard.flush()
while(1):
	msg = ard.readline()	
	#time.sleep(1.5)
	if(msg):
		print msg
		msg2 = int(msg)

		if(msg2==1001):
			print ('writing val for 1001')
			val = sheet['B2'].value
			val = val + 10
			print(val)
			sheet['B2'] = val
			msg = '0'

		elif(msg2==1002):
			print ('writing val for 1002')
			val = sheet['B3'].value
			val = val + 10
			sheet['B3'] = val
			msg = '0'
		elif(msg2==1011):
			print ('writing val for 1011')
			val = sheet['B2'].value
			val = val - 10
			sheet['B2'] = val
			msg = '0'
		elif(msg2==1012):
			print ('writing val for 1011')
			val = sheet['B3'].value
			val = val - 10
			sheet['B3'] = val
			msg = '0'
		log.save("DataLog.xlsx")
