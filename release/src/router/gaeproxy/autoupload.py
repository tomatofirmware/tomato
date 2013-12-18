#!/usr/bin/env python

import pexpect
import datetime  
import time  
import os  
import re  

MYAPPIDS = [ 'app1', 'app2', 'app3' ]
EMAIL = 'yyyy@gmail.com'
PASSWD = 'xxxxxxxx'

WPSVRHOME = '/usr/lib/python2.7/site-packages/gaeproxy/server'

try:
	print "--------------------------------------"
	for i,APPID in enumerate(MYAPPIDS):
		print "Now uploading to applicaion: " + APPID
		if os.path.isfile(os.path.join(WPSVRHOME, ".appcfg_cookies")) == True :
			os.remove (os.path.join(WPSVRHOME, ".appcfg_cookies"))
		
		cmd = '/usr/bin/python ' + WPSVRHOME + '/uploader.py'
		child = pexpect.spawn(cmd)
		print "SPAWN OK." + cmd
		index = child.expect( ['make sure your wallproxy is running: ' , pexpect.EOF, pexpect.TIMEOUT] )
		if index == 0 :
			child.sendline("0")
			index = child.expect( ['APPID:', pexpect.EOF, pexpect.TIMEOUT] )
			if index == 0 :
				print "Send APPID." 
				child.sendline(APPID)
				index = child.expect( ['Email: ', pexpect.EOF, pexpect.TIMEOUT] )
				if index == 0 :
					print "Send Email."
					child.sendline(EMAIL)
					index = child.expect( ["Password for*", pexpect.EOF, pexpect.TIMEOUT])
					if index == 0 :
						print "Send Password."
						child.sendline(PASSWD)
						time.sleep(10)
						index = child.expect( ['Completed update of app:*', 'Invalid username or password*', "This application does not exist*", 'Email: ', pexpect.EOF, pexpect.TIMEOUT] )
						if index == 0 :
							print "Successfully upload file(1): " + APPID
							child.close(force=True)
						elif index == 1 :
							print "Invalid username or password(1)."
							child.close(force=True)
						elif index == 2 :
							print "Invalid APPID(1)."
							child.close(force=True)
						elif index == 3 :
							print "Send Email again."
							child.sendline(EMAIL)
							index = child.expect( ["Password for*", pexpect.EOF, pexpect.TIMEOUT])
							if index == 0 :
								print "Send Password again."
								child.sendline(PASSWD)
								time.sleep(10)
								index = child.expect( ['Completed update of app:*', 'Invalid username or password*', "This application does not exist*", pexpect.EOF, pexpect.TIMEOUT] )
								if index == 0 :
									print "Successfully upload file(2): " + APPID
									child.close(force=True)
								elif index == 1 :
									print "Invalid username or password(2)."
									child.close(force=True)
								elif index == 2 :
									print "Invalid APPID(2)."
									child.close(force=True)
								else:
									print "Upload failed(2), due to TIMEOUT or EOF whening waiting uploader completion."
									child.close(force=True)
							else:
								print "Upload failed, Send Email again failed."
								child.close(force=True)
						else:
							print "Upload failed, due to TIMEOUT or EOF whening waiting uploader completion."
							child.close(force=True)
					else:
						print "Upload failed, due to TIMEOUT or EOF when waiting Password."
						child.close(force=True)
				else:
					print "Upload failed, due to TIMEOUT or EOF when waiting Email."
					child.close(force=True)
			else:
				print "Upload failed, due to TIMEOUT or EOF when waiting APPID."
				child.close(force=True)
		else:
			print "Upload failed, due to TIMEOUT or EOF when waiting select config.."
			child.close(force=True)
			
		if os.path.isfile(os.path.join(WPSVRHOME, ".appcfg_cookies")) == True :
			os.remove (os.path.join(WPSVRHOME, ".appcfg_cookies"))
		print "--------------------------------------"
	else:
		print "All of APPIDs have been uploaded."
		
except Exception, ex:
	print ex
	
