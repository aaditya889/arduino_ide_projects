from django.urls import path
from . import views


urlpatterns = [
    path('nodemcu', views.consume_nodemcu, name='consume_nodemcu'),
]
