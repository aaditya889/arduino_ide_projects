from django.urls import path
from . import views


urlpatterns = [
    path('get_data', views.get_data, name='get_data'),
    path('set_data', views.set_data, name='set_data'),
]
