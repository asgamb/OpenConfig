#!/usr/bin/env python

from ncclient import manager
import time
from lxml import etree

SLEEP = 0.15
init = 0
append = 0


class Lumentum:
    def __init__(self, repeat, ip):
        self.repeat = repeat
        self.ip = ip
        self.handle = None


    def getData(self):
        with manager.connect(host=self.ip,
                             port=830,
                             username='superuser',
                             password='Sup%9User',
                             hostkey_verify=False,
                             # device_params={'name': 'lumentum'},
                             allow_agent=False,
                             look_for_keys=False) as lumentum_manager:
            treefilter = '''
                     <connections xmlns="http://www.lumentum.com/lumentum-ote-connection">
                     </connections>
                     '''
            if self.repeat == 1:
                while (1):
                    reply = lumentum_manager.get(('subtree', treefilter))
                    self.process_reply(reply)
                    time.sleep(SLEEP)
            else:
                reply = lumentum_manager.get(('subtree', treefilter))
                datas = self.process_reply(reply)
                return datas

    def getValues(self):
            treefilter = '''
                     <connections xmlns="http://www.lumentum.com/lumentum-ote-connection">
                     </connections>
                     '''
            if self.repeat == 1 and self.handle is not None:
                while (1):
                    reply = self.handle.get(('subtree', treefilter))
                    self.process_reply(reply)
                    time.sleep(SLEEP)
            else:
                reply = self.handle.get(('subtree', treefilter))
                datas = self.process_reply(reply)
                return datas


    def connect(self):
        """ Establish netconf session """

        if self.handle is not None:
            return True

        try:
            # timeout is configurable as environment variable
            self.handle = manager.connect(host=self.ip,
                                          port=830,
                                          username='superuser',
                                          password='Sup%9User',
                                          allow_agent=False,
                                          look_for_keys=False,
                                          hostkey_verify=False)
        except:
            print("Failed to create netconf session:")
            self.handle = None
            return False

        print("Connected: %s" % self.__str__())
        return True 


    def process_reply(self, reply):
        reply3 = str(reply).encode('utf-8')
        root = etree.fromstring(reply3)

        dns = []
        in_powers = []
        out_powers = []
        atts = []
        for data in root:
            if data.tag == "{urn:ietf:params:xml:ns:netconf:base:1.0}data":
                for connections in data:
                    if connections.tag == "{http://www.lumentum.com/lumentum-ote-connection}connections":
                        for connection in connections.findall(
                                '{http://www.lumentum.com/lumentum-ote-connection}connection'):
                            dn = connection.find('{http://www.lumentum.com/lumentum-ote-connection}dn')
                            dnx = dn.text
                            dnt = dnx.split('=')
                            dns.append(dnt[5])
                            for state in connection:
                                if state.tag == "{http://www.lumentum.com/lumentum-ote-connection}state":
                                    att = state.find('{http://www.lumentum.com/lumentum-ote-connection}attenuation')
                                    atts.append(att.text)
                                    for child in state:
                                        if child.tag == "{http://www.lumentum.com/lumentum-ote-connection}input-channel-attributes":
                                            in_power = child.find(
                                                '{http://www.lumentum.com/lumentum-ote-connection}power')
                                            in_powers.append(in_power.text)
                                        if child.tag == "{http://www.lumentum.com/lumentum-ote-connection}output-channel-attributes":
                                            out_power = child.find(
                                                '{http://www.lumentum.com/lumentum-ote-connection}power')
                                            out_powers.append(out_power.text)
        # self.save_data(dns, atts, in_powers, out_powers, 0)
        conns = {}
        for i in range(0, len(dns)):
            conns[dns[i]] = {}
            conns[dns[i]]['input'] = in_powers[i]
            conns[dns[i]]['output'] = out_powers[i]
        return conns

    def save_data(self, dns, atts, in_powers, out_powers, save):

        global init
        global append
        print("id->" + str(dns))
        print("atts->" + str(atts))
        print("in->" + str(in_powers))
        print("out->" + str(out_powers))
        print("#########################")
        millis = int(round(time.time() * 1000))
        if save == 1:
            for count in range(len(dns)):
                if (append == 1):
                    if (init == 0):
                        input_file = open("connection" + dns[count] + ".txt", "a")
                        # input_file.write("attenuation,input_power,output_power\n")
                        input_file.write("timestamp\tinput_power\n")
                        # input_file.write(atts[count]+"\t"+in_powers[count]+"\t"+out_powers[count]+"\n")
                        input_file.write(str(millis) + "\t" + in_powers[count] + "\n")
                        input_file.close()
                    else:
                        input_file = open("connection" + dns[count] + ".txt", "a")
                        # input_file.write(atts[count]+"\t"+in_powers[count]+"\t"+out_powers[count]+"\n")
                        input_file.write(str(millis) + "\t" + in_powers[count] + "\n")
                        input_file.close()
                else:
                    if (init == 0):
                        input_file = open("connection" + dns[count] + ".txt", "w+")
                        # input_file.write("attenuation,input_power,output_power\n")
                        # input_file.write(atts[count]+"\t"+in_powers[count]+"\t"+out_powers[count]+"\n")
                        input_file.write("timestamp\tinput_power\n")
                        input_file.write(str(millis) + "\t" + in_powers[count] + "\n")
                        input_file.close()
                    else:
                        input_file = open("connection" + dns[count] + ".txt", "a")
                        input_file.write(str(millis) + "\t" + in_powers[count] + "\n")
                        input_file.close()
                init = 1
#l1 = Lumentum(0, '10.10.10.30')
#a=l1.getValues()
#inpow=a['10']['input']
#outpow=a['11']['output']

#print(inpow, outpow)
