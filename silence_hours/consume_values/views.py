from django.shortcuts import render
from django.http import JsonResponse
from rest_framework.decorators import api_view
from rest_framework import status
from rest_framework.response import Response
import mysql.connector as mysql
from .poller import poller
import pytz
from datetime import datetime, timezone, timedelta
import threading
import time

db = mysql.connect(
    host = "localhost",
    user = "root",
    # passwd = "asdfg12345",
    passwd = "aaditya",
    database = "pod_assistant"
)

cursor = db.cursor()
# cursor.execute("CREATE DATABASE pod_assistant")
cursor.execute("""CREATE TABLE IF NOT EXISTS audio (
    id INT AUTO_INCREMENT PRIMARY KEY, 
    pod_name VARCHAR(255), 
    value INT, 
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, 
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)""")

INSERT_IN_AUDIO_QUERY = 'INSERT INTO audio (pod_name, value) VALUES ("{}", {})'
GET_LATEST_AUDIO_QUERY = 'select * from audio where created_at >= DATE_SUB("{}", INTERVAL {} SECOND)'
GET_AUDIO_QUERY = 'select * from audio where created_at >= {}'
ALERT_WAIT_TIME_IN_MINS = 5
ALERT_THRESHOLD_AUDIO_VALUE = 10
AUDIO_DATA_FETCH_TIME_RANGE_SECS = 30
POLLER_WAIT_TIME_SECS = 2


################# UTILS #########################################
def convert_time_utc_to_mysql_format(datetime_object):
    # return str(datetime_object.astimezone(pytz.timezone('Asia/Kolkata'))).split('.')[0]
    return time.strftime("%Y-%m-%d %H:%M:%S")
 
def get_current_time():
    return convert_time_utc_to_mysql_format(datetime.now())


################# DMLS #########################################
def get_latest_audio_data(time_in_secs):
    query = GET_LATEST_AUDIO_QUERY.format(get_current_time(), time_in_secs)
    print(query)
    cursor.execute(query)
    records = cursor.fetchall()
    return records

# start time to be in datetime object
def get_audio_data(start_time):
    query = GET_AUDIO_QUERY.format(convert_time_utc_to_mysql_format(start_time))
    print(query)
    cursor.execute(query)
    records = cursor.fetchall()
    return records

#################### APIs #######################################

@api_view(['POST'])
def get_data(request):
    print(request.data)
    #TODO: convert start time to datetime
    result = {}#get_audio_date(request.data['start_time'])
    return JsonResponse(result, status=status.HTTP_200_OK)

@api_view(['POST'])
def set_data(request):

    print(request.data)

    query = INSERT_IN_AUDIO_QUERY.format(request.data['pod_name'], request.data['value'])
    cursor.execute(query)
    db.commit()
    return JsonResponse({}, status=status.HTTP_200_OK)

############## worker ###########################################
def construct_next_poll(response):
    next_poll = dict()
    next_poll['status'] = 'success'
    next_poll['retry'] = True
    next_poll['request'] = response['updated_request']
    return next_poll

def get_avg_audio_value(data):
    total_value = 0
    for audio_data in data:
        total_value = total_value + audio_data[2]
    return total_value/(len(data) or 1)

@poller(wait_time_secs=POLLER_WAIT_TIME_SECS, construct_next_poll=construct_next_poll)
def alert_on_noise(last_alert_time, threshold_audio_value, time_range_in_secs):
    response = dict()
    response['data'] = get_latest_audio_data(time_range_in_secs)
    avg = get_avg_audio_value(response['data'])
    print("average value: ", avg)

    time_spent_since_last_alert = datetime.now() - last_alert_time
    if avg > threshold_audio_value and time_spent_since_last_alert.seconds / 60 > ALERT_WAIT_TIME_IN_MINS:
        print("SENDING ALERT")
        #send_slack_alert()
        #send_voice_alert()
        last_alert_time = datetime.now()

    response['updated_request'] = last_alert_time
    print("worker fetch count: ", len(response['data']))
    return response

worker = threading.Thread(target=alert_on_noise, args=(datetime.now() - timedelta(minutes=ALERT_WAIT_TIME_IN_MINS), ALERT_THRESHOLD_AUDIO_VALUE, AUDIO_DATA_FETCH_TIME_RANGE_SECS))
worker.start()


@api_view(['GET'])
def test(request):

    return JsonResponse({"hello":"world"}, status=status.HTTP_200_OK)