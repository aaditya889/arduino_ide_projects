# Details about the drone

## Write Operations

### Functions calling update_esc_power()

1. update_thrust_vector() [drone.h]
2. calibrate_esc() [pwm.h]
<!-- 3. find_com_thrust_ratio() [trim.h] -->

### Functions changing YPR

1. complementary_filter() [drone.h]

## Read Operations

### Functions reading YPR

1. complementary_filter() [drone.h]
2. update_thrust_vector() [drone.h]
3. export_drone_stats() [drone.h]
4. find_com_thrust_ratio() [trim.h]
