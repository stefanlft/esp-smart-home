#!/bin/bash

# Configurable parameters
ROOM="living_room"
NODE="node2"
URL="http://localhost:8080/update"

# Simulate sending data every 5 seconds
while true; do
    TEMP=$((RANDOM % 10 + 20))  # random temp between 20-29
    HUM=$((RANDOM % 30 + 40))   # random humidity between 40-69
    LIGHT=$((RANDOM % 100)) # random lightness between 0-99
    MOTION=$((RANDOM % 2))      # 0 or 1 (boolean)

    curl -X POST -d "room=$ROOM" -d "node=$NODE" \
         -d "temperature=$TEMP" \
         -d "humidity=$HUM" \
         -d "lightness=$LIGHT" \
         -d "motion=$MOTION" \
         $URL

    echo "Sent data: Temp=$TEMP, Hum=$HUM, Light=$LIGHT, Motion=$MOTION"
    sleep 5
done
