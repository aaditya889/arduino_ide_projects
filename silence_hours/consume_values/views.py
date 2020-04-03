from django.shortcuts import render
from django.http import JsonResponse
from rest_framework.decorators import api_view
from rest_framework import status
from rest_framework.response import Response
import mysql.connector as mysql
from .poller import poller
import pytz
from datetime import datetime, timezone
import threading

db = mysql.connect(
    host = "localhost",
    user = "root",
    passwd = "asdfg12345",
    database = "pod_assistant"
)

cursor = db.cursor()
#cursor.execute("CREATE DATABASE pod_assistant")
cursor.execute("""CREATE TABLE IF NOT EXISTS audio (
    id INT AUTO_INCREMENT PRIMARY KEY, 
    pod_name VARCHAR(255), 
    value INT, 
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, 
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)""")

INSERT_IN_AUDIO_QUERY = 'INSERT INTO audio (pod_name, value) VALUES ("{}", {})'
GET_LATEST_AUDIO_QUERY = 'select * from audio where created_at >= DATE_SUB("{}", INTERVAL {} SECOND)'
GET_AUDIO_QUERY = 'select * from audio where created_at >= {}'

################# UTILS #########################################
def convert_time_utc_to_mysql_format(datetime_object):
    return str(datetime_object.astimezone(pytz.timezone('Asia/Kolkata'))).split('.')[0]
 
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

@poller(wait_time_secs=2, construct_next_poll=construct_next_poll)
def alert_on_noise(last_alert_time, threshold_audio_value, time_range_in_secs):
    response = dict()
    response['data'] = get_latest_audio_data(time_range_in_secs)
    response['updated_request'] = last_alert_time
    print("worker fetch count: ", len(response['data']))
    return response

worker = threading.Thread(target=alert_on_noise, args=(None, None, 60))
worker.start()

