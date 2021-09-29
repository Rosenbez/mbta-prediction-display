#pragma once
#include <Arduino.h>
/* 
example of the data layout
data : [
  {'attributes': {'arrival_time': '2021-03-07T19:08:44-05:00',
    'departure_time': '2021-03-07T19:08:44-05:00',
    'direction_id': 1,
    'schedule_relationship': None,
    'status': None,
    'stop_sequence': 6},
   'id': 'prediction-46554305-2579-6',
   'relationships': {'route': {'data': {'id': '88', 'type': 'route'}},
    'stop': {'data': {'id': '2579', 'type': 'stop'}},
    'trip': {'data': {'id': '46554305', 'type': 'trip'}},
    'vehicle': {'data': {'id': 'y1936', 'type': 'vehicle'}}},
   'type': 'prediction'},
   ]
*/


class StopPrediction
{
private:
    const char *_route;
    const char *_stop;
    const char *_arrival_time;
    const char *_via;

public:
    StopPrediction(const char *route, const char *stop, const char *arrival_time) : _route(route), _stop(stop), _arrival_time(arrival_time) {}
    StopPrediction() : _route("0"), _stop("0"), _arrival_time("0"), _via("0") {}
    const char *route() { return _route; }
    const char *stop() { return _stop; }
    const char *arrival_time() { return _arrival_time; }
    const char *via() { return _via; }
    const int get_countdown_min(struct tm &current_time)
    {
        String arrival = _arrival_time; // format "2021-03-07T09:30:51-05:00"
        Serial.println(arrival);
        int hr = arrival.substring(11, 13).toInt();
        Serial.printf("Got arrival hr: %d, ", hr);
        int minute = arrival.substring(14, 16).toInt();
        Serial.printf("Got arrival min: %d \n", minute);
        int arriving_in = (hr - current_time.tm_hour) * 60 + (minute - current_time.tm_min);
        return arriving_in;
    }
    void set_via(const char *via) { _via = via; }

    bool hasVia()
    {
        return !strcmp(_via, "0") == 0;
    }
};

struct PredictionPack {
    StopPrediction predictions[15];
    int num_predictions;
};
