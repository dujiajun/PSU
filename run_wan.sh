#!/bin/bash 

tcdel lo --all
tcset lo --rate 400Mbps --delay 40ms --network 127.0.0.1 --port 12345
bash ./run.sh wan
tcdel lo --all