import os
#os.system("python /home/mininet/confd/bin/netconf-console --proto=tcp --port=2023 --user=admin --password=admin --edit-config '/home/mininet/confd/examples.confd/transponderVCSL/set_create_connection.xml'")

import signal
import sys

CREATE_CONNECTION = "python /home/mininet/confd/bin/netconf-console --proto=tcp --port=2023 --user=admin --password=admin --edit-config '/home/mininet/confd/examples.confd/transponderVCSL/set_create_connection.xml'"
RECONFIGURE_CONN = "python /home/mininet/confd/bin/netconf-console --proto=tcp --port=2023 --user=admin --password=admin --edit-config '/home/mininet/confd/examples.confd/JOCN_Transponder/reconfigure_connection.xml'"
SUBSCRIPTION = "python /home/mininet/confd/bin/netconf-console --proto=tcp --port=2023 --user=admin --password=admin -s all '/home/mininet/confd/examples.confd/JOCN_Transponder/sub1.xml'"


files = []

def signal_handler(signal, frame):
        print('Safe exit. Bye')
        map(lambda f: f.close(),files) #close all files
        sys.exit(0)

#attach hendler to catch Ctrl+C
signal.signal(signal.SIGINT, signal_handler)


# run create connection command
os.system(CREATE_CONNECTION)

# run subscription command
f = os.popen(SUBSCRIPTION)
files.append(f)
while (True):
	s = f.readline()
	print s
	# if ber over threshold run connection reconfiguration
	if "pre-fec-ber-change" in s:
		os.system(RECONFIGURE_CONN)



#prevent script to exit until Ctrl+C is hit
#signal.pause()


'''
import threading
import subprocess

class NotificationListener(threading.Thread):
    def __init__(self):
        self.stdout = None
        self.stderr = None
        threading.Thread.__init__(self)

    def run(self):
        f = os.popen(SUBSCRIPTION)
		files.append(f)
		while (True):
			s = f.readline()
			print s
			if "ber-threshold" in s:
				os.system(RECONFIGURE_CONN)



myclass = MyClass()
myclass.start()
myclass.join()
'''


#import subprocess

#proc = subprocess.Popen(["python", "/home/mininet/confd/bin/netconf-console --proto=tcp --port=2023 --user=admin --password=admin --edit-config '/home/mininet/confd/examples.confd/transponderVCSL/set_create_connection.xml'"], stdout=subprocess.PIPE, shell=True)
#(out, err) = proc.communicate()
#print "program output:", out