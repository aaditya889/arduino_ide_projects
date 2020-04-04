import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import requests
from datetime import datetime

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
API_URL = "http://localhost:8000/consume/get_latest_data"
API_PARAMS = {
  'time_in_secs': 60
}


def animate(i):
  curr_time = datetime.now()
  response = requests.post(url=API_URL, data=API_PARAMS)
  pullData = response.json()
  print(pullData)
  xar = []
  yar = []
  for eachRow in pullData['data']:
    then_time = datetime.strptime(eachRow[3], '%Y-%m-%dT%H:%M:%S')
    diff_in_secs = (curr_time - then_time).total_seconds()
    x = diff_in_secs
    y = eachRow[2]
    print("x", diff_in_secs)
    print("y", eachRow[2])
    xar.append(int(x))
    yar.append(int(y))
  ax1.clear()
  ax1.plot(xar,yar)
  API_PARAMS['time_in_secs'] = 30


ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()
