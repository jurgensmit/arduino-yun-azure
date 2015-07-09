#  Arduino Yun Azure

Connect the Arduino Yun to Azure. A couple of sensors are connected to the Arduino Yun. Every few seconds the sensor values are read and sent to an Azure EventHub. The EventHub is using streaming analytics to average sensor values each 15 minutes into an Azure table.
