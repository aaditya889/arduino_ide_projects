from django.shortcuts import render
from django.http import JsonResponse
from rest_framework.decorators import api_view
from rest_framework import status
from rest_framework.response import Response
import mysql.connector as mysql

db = mysql.connect(
    host = "localhost",
    user = "root",
    passwd = "asdfg12345"
)

cursor = db.cursor()
#cursor.execute("CREATE DATABASE pod_assistant")

print("db conn: ",db)

@api_view(['POST'])
def get_data(request):

    print(request.data)
    x = {
        "shashank": "sah"
    }
    return JsonResponse(x, status=status.HTTP_200_OK)
