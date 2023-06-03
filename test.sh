#!/bin/bash

pid=$(ps aux | grep "[w]ebserv" | awk '{print $2}')

while [ 1 ]; do leaks ${pid} | grep "total leaked bytes."; sleep 1; done
