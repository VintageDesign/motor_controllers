# Motor Controllers
C++ Component for ESP32 to controll various motors

## Setup:
Per the ESP IDF doc, add this project as a submodule to your project in the
`components/` dir.

## Linking
To link to your project, add `motor_controllers` to your IDF requirements:
```cmake
idf_component_register(...Your Project... REQUIRES motor_controllers)
```


