import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import requests
from datetime import datetime

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
API_URL = "http://localhost:8000/consume/get_latest_data"
API_PARAMS = {
  'time_in_secs': 900000
}

def animate(i):
  curr_time = datetime.now()
  response = requests.post(url=API_URL, data=API_PARAMS)
  pullData = response.json()
  xar = []
  yar = []
  for eachRow in pullData['data']:
    timestamp = eachRow[3]
    noise_level = eachRow[2]
    xar.append(timestamp)
    yar.append(int(noise_level))
  ax1.clear()
  ax1.plot(xar,yar)
  plt.xlabel('time', fontsize=10)
  plt.ylabel('noise level', fontsize=10)
  plt.xticks(rotation=30)
  plt.xticks(xar, "")
  plt.title("NOISE TREND")


ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()
