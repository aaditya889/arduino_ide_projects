import matplotlib.pyplot as plt
import time
import requests
from datetime import datetime
import numpy as np


API_URL = "http://localhost:8000/consume/get_data_with_time_range"
API_PARAMS = {
  'start_time': '2020-04-04 14:30:00',
  'end_time': '2020-04-04 15:05:00'
}

response = requests.post(url=API_URL, data=API_PARAMS)
pullData = response.json()

time_values = []
noise_values = []
pod_names = []
for eachRow in pullData['data']:
  pod_names.append(eachRow[1])
  time_values.append(eachRow[3])
  noise_values.append(eachRow[2])

distinct_pod_names = set(pod_names)

hl, = plt.plot(time_values, noise_values)
plt.xlabel('time', fontsize=10)
plt.ylabel('noise level', fontsize=10)
plt.title(distinct_pod_names.pop().upper() + ' NOISE TREND', fontsize=15)
plt.xticks(rotation=30)
plt.show()
