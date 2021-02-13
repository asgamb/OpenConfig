1.  Information on NETCONF agent

    download confd from website (we suggest to use version 6.4 of confd)

    then unzip the folder and run
    sh confd-basic-6.4.linux.i686.installer.bin /home/username/confd

    cp the OpenConfigRW folder in /home/username/confd/example.confd/ folder
    
    then move to /home/username/confd/example.confd/OpenConfigRW folder and run
    
    export CONFD_DIR=/home/username/confd/
    chmod 777 startNetconfAgent.sh
    
    edit the parameters.h file changing the LOCALPATH variable value /home/username/confd/bin/
    ./startNetconfAgent.sh


    1.1 Test of NETCONF agent
        To test NETCONF capability we can use the client provided with confD in /home/username/confd/bin
        
        Requirements:
        install python pip
        install ssh paramiko:
        pip install paramiko (for NETCONF protocol over ssh)
        
        install xmllint
        apt-get install libxml2-utils
        
        cd /home/username/confd/bin
        
        and run the NETCONF client commands:
        #get-config
        netconf-console --port=2023 --proto=tcp --get-config -x '/terminal-device'
        
        #edit-config
        netconf-console --port=2023 --proto=tcp --edit-config 'editOC.xml'

2. Information on the driver (requirement Python 2.7)

    In the driver folder there is a python script to activate the interacaction with external devices.
    
    Two are the main functions of the driver:
    -)receiving the transponder configuration
    -)report the monitoring values
    
    Looking at the file ExampleDriverMH.py, two sockets are adopted to perform the communication with the Netconf agent regarding configuration and the monitoring.
    
    The function loadByKey() is the one used to report monitoring values. At the moment we implemented "PRE-FEC-BER","ESNR","Q-VALUE", "FREQUENCY", "CHROMATIC-DISPERSION" parameters.
    
    The syntax uses the "###" as separator and includes:
            - logical channel id or component name +###
            - key reported
            - instant value +###
            - average value +###
            - min value +###
            - Max value+&&
    The string "&&" is used to close the communication.
    
    I put in this function all the implemented values as example.
    Chromatic-dispersion and frequency parameters refer to physical-components that are indexed in the model by name with the format id/shelf/slot/port.
    The other parameters refer to logical-channels. Since the logical channels are indexed in the model by id (an integer) we considered to have the following mapping on the integer id: shelf(1 digit)slot(2digits)port(2digits).
    
    Regarding the configuration you can refer to function listenerConf() where the parser of the command raised by the netconf agent after an edit-config is shown.
    In this example, after receiving all the configuration parameters, a text configuration file is generated.


3. Run a test with driver and agent

    Configuration:
    -)Agent:
      edit /home/username/confd/example.confd/OpenConfigRW/parameters.h
       1)change the value of CONF_TRANSPONDER to 1;
       2)change the value of TRANSPONDER_ADDR to the IP of the machine where the driver is running
    
    -)Driver:
      edit the driver script ExampleDriverMH.py
      1)set at line 10 the IP value of the machine where the netconf agent is running
     
    Then, run the netconf agent and the driver.
    When the 2 sockets are connected (configuration and monitoring) send the edit-config of the previous example
    the driver will receive the configuration, saving it in the Config.txt file
    
    The driver will periodically report the values of the monitored parameters.

4. GRPC installation
To install the GRPC functionality, install basic repositories:

  python -m pip install --upgrade pip
  python -m pip install grpcio
  python -m pip install grpcio-tools

Copy the folder grpc in the path
/home/username/
 (home/username/grpc)
    

5. Run a test with agent, telemetry and driver

    Configuration:
    -)Agent:
      edit /home/username/confd/example.confd/OpenConfigRW/parameters.h
       1)change the value of CONF_TRANSPONDER to 1;
       2)change the value of TRANSPONDER_ADDR to the IP of the machine where the driver is running
	   3)nable the GRPC feature by setting to 1 the ENABLE_GRPC value
	   4)set the GRPCPATH value to "/home/username/grpc/")
	   5)set the GRPC_SERVER_IP value to local ip
	   6)set the GRPC_SERVER_PORT value configuring the desired port
	   7)set the path to netconf-console python file (according yo confd/bin dir in the files:
	      server_telemetry3.0.py and subscribeTelemetry3.0.py
	   
    -)Driver:
      edit the driver script ExampleDriverMH.py
      1)set at line 10 the IP value of the machine where the netconf agent is running
     
    Then, run the netconf agent and the driver.
	
	An example of subscription is provided with the editTelemetry.xml file.
	
	Start the client application provided with the GRPC software (OCTelementryClient.py).
	
	Before a GRPC subscription, edit this file setting:
		- the <destination-address>
		- the <destination-port>
        - the <sample-interval> (duration of the subscription)
		- the <heartbeat-interval> (interval between 2 samples)
		- the <suppress-redundant> (to avoid the receiption of duplicated valies)
	
    In the telemetry subscription example the instant pre-fec-ber is included (more path che be added to teh list) 	
	
	Once ready, put the editTelemetry.xml file in /home/username/confd/bin

	cd /home/username/confd/bin	
	and run
	python netconf-console --port=2023 --proto=tcp --edit-config 'editTelemetry.xml'
	
	You will receive key value data that will be printed by the GRPC client.
	
	
