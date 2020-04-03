from django.shortcuts import render
from django.http import JsonResponse
from rest_framework.decorators import api_view
from rest_framework import status
from rest_framework.response import Response


@api_view(['POST'])
def consume_nodemcu(request):

    print(request.data)
    x = {
        "shashank": "sah"
    }
    return JsonResponse(x, status=status.HTTP_200_OK)
