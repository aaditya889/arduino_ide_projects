from django.shortcuts import render
from django.http import JsonResponse
from rest_framework.decorators import api_view
from rest_framework import status
from rest_framework.response import Response
import mysql.connector as mysql

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
    cerated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)""")

INSERT_IN_AUDIO_QUERY = 'INSERT INTO audio (pod_name, value) VALUES ("{}", {})'

print("db conn: ",db)

@api_view(['POST'])
def get_data(request):

    print(request.data)
    x = {
        "shashank": "sah"
    }
    return JsonResponse(x, status=status.HTTP_200_OK)

@api_view(['POST'])
def set_data(request):

    print(request.data)

    query = INSERT_IN_AUDIO_QUERY.format(request.data['pod_name'], request.data['value'])
    cursor.execute(query)
    db.commit()
    return JsonResponse({}, status=status.HTTP_200_OK)
