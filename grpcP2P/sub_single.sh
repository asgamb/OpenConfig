#!/bin/bash


p1=$1 #duration
p2=$2 #template_id
p3=$3 #subscription_id

#number of instances
if [ -z "$1" ]
then
    p1=100000
fi

if [ -z "$2" ]
then
    p2=2
fi

if [ -z "$3" ]
then
    p3=11
fi

echo "Creating subscriptions"

for j in {1..1}
do
  #sudo docker stop -t 1 c$j
  id="c"$j
  echo $id
  if [[ $j -gt 9 ]]
  then
    ip="10.10.255.2"$j
  else
    ip="10.10.255.20"$j
  fi
  python subscriber.py --server 10.10.255.201 --keys input,output --observ amplifier2 --duration $p1 --ip 10.30.2.37 --group $p2 --register $j

done

#python subscriber.py --server 10.10.255.202 --keys spo --observ card2 --duration $p1 --ip 10.10.255.1 --group $p2 --register $p3
#python subscriber.py --server 10.10.255.101 --keys lumentum --observ lumentum --duration $p1 --ip 10.10.255.1 --group $p2 --register $p3
#python subscriber.py --server 10.10.255.102 --keys input,output --observ amplifier1 --duration $p1 --ip 10.10.255.1 --group $p2 --register $p3
#python subscriber.py --server 10.10.255.103 --keys input,output --observ amplifier2 --duration $p1 --ip 10.10.255.1 --group $p2 --register $p3
echo "done"


