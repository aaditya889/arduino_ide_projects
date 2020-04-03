import time

def poller(wait_time_secs, construct_next_poll):
    def decorator(func):
        def wrapper(request, *args, **kwargs):
            while(True):
                response = func(request, *args, **kwargs)
                next_poll = construct_next_poll(response)
                if not next_poll['retry']:
                    return response
                if 'request' in next_poll:
                    request = next_poll['request']
                time.sleep(wait_time_secs)
        return wrapper
    return decorator