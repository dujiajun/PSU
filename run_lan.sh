#!/bin/bash 
# 
# tc uses the following units when passed as a parameter. 
# kbps: Kilobytes per second 
# mbps: Megabytes per second 
# kbit: Kilobits per second 
# mbit: Megabits per second 
# bps: Bytes per second 
#  Amounts of data can be specified in: 
#  kb or k: Kilobytes 
#  mb or m: Megabytes 
#  mbit: Megabits 
#  kbit: Kilobits 
# To get the byte figure from bits, divide the number by 8 bit 
# 

# The network interface we're planning on limiting bandwidth. 
IF='lo'

# Latency 
LAT_1=0.08ms   # Base latency 
LAT_2=50ms   # Plus or minus 
LAT_3=25%   # Based on previous packet % 
# Dropping packets 
DROP_1=5%   # Base probability 
DROP_2=25%   # Based on previous packet % 
# Bandwidth 
DNLD=10Gbit   # DOWNLOAD Limit 
UPLD=10Gbit   # UPLOAD Limit 

# IP address of the machine we are controlling 
IP="127.0.0.1" 

# Filter options for limiting the intended interface. 
U32="tc filter add dev $IF protocol ip parent 1:0 prio 1 u32" 

# We'll use Hierarchical Token Bucket (HTB) to shape bandwidth. 
# For detailed configuration options, please consult Linux man 
# page. 
tc qdisc del dev $IF root
#$TC qdisc add dev $IF root handle 2: netem delay $LAT_1 $LAT_2 $LAT_3  loss $DROP_1 $DROP_2 
tc qdisc add dev $IF root handle 2: netem delay $LAT_1
tc qdisc add dev $IF parent 2: handle 1: htb default 30 
tc class add dev $IF parent 1: classid 1:1 htb rate $DNLD 
tc class add dev $IF parent 1: classid 1:2 htb rate $UPLD 
$U32 match ip dst $IP/32 flowid 1:1 
$U32 match ip src $IP/32 flowid 1:2 

# The first line creates the root qdisc, and the next three lines 
# create three child qdisc that are to be used to shape download 
# and upload bandwidth. 
# 
# The 5th and 6th line creates the filter to match the interface. 
# The 'dst' IP address is used to limit download speed, and the 
# 'src' IP address is used to limit upload speed. 

echo "Limit to $DNLD on $IF for $IP" 

bash ./run.sh lan

tc qdisc del dev $IF root