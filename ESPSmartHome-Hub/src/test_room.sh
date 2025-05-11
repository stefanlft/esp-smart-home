curl -X POST "http://127.0.0.1:8080/update" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  --data-urlencode "room=bedroom" \
  --data-urlencode "node=alpha" \
  --data-urlencode "lightness=75%" \
  --data-urlencode "temperature=30°C" \
  --data-urlencode "humidity=50%" \
  --data-urlencode "motion=0"
