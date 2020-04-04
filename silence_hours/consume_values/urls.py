from django.urls import path
from . import views


urlpatterns = [
    path('get_latest_data', views.get_latest_data, name='get_latest_data'),
    path('get_data_with_time_range', views.get_data_with_time_range, name='get_data_with_time_range'),
    path('set_data', views.set_data, name='set_data'),
    path('end_silence_hours', views.end_silence_hours, name='end_silence_hours'),
    path('start_silence_hours', views.start_silence_hours, name='start_silence_hours'),
]

# curl localhost:8000/consume/get_latest_data -XPOST -H 'Content-Type: application/json' -d'{"time_in_secs":60}'
# curl localhost:8000/consume/get_data_with_time_range -XPOST -H 'Content-Type: application/json' -d'{"start_time":"2020-04-04 01:22:28", "end_time" : "2020-04-04 13:42:28"}'
# curl localhost:8000/consume/set_data -XPOST -H 'Content-Type: application/json' -d'{"pod_name":"platform", "value": 15}'
# curl localhost:8000/consume/start_silence_hours -XPOST -H 'Content-Type: application/json' -d'{}'
# curl localhost:8000/consume/end_silence_hours -XPOST -H 'Content-Type: application/json' -d'{}'