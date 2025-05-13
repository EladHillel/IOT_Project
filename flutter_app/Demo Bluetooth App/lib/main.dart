import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';

void main() async {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Bluetooth Flutter App',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  BluetoothConnection? connection;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: ElevatedButton(
          onPressed: _connect,
          child: Text('Find BT Device'),
        ),
      ),
    );
  }

  void _connect() async {
    BluetoothDevice? selectedDevice = await Navigator.push(
      context,
      MaterialPageRoute(builder: (context) => DeviceListPage()),
    );

    if (selectedDevice != null) {
      try {
        connection =
            await BluetoothConnection.toAddress(selectedDevice.address);
        // connection = await BluetoothConnection.toAddress('B0:A7:32:DB:0C:9A'); //try using this if you don't want to select the device every time
        // incoming messages will be handled by _onDataReceived

        print('Connected to ${selectedDevice.name}');

        if (connection != null) {
          Navigator.push(
            context,
            MaterialPageRoute(
                builder: (context) => CheckScreen(connection: connection!)),
          );
        }
      } catch (e) {
        print('Error connecting: $e');
      }
    }
  }
}

class DeviceListPage extends StatelessWidget {
  Future<List<BluetoothDevice>> _getBondedDevices(BuildContext context) async {
    List<BluetoothDevice> devices = [];
    try {
      devices = await FlutterBluetoothSerial.instance.getBondedDevices();
    } catch (e) {
      print('Error getting bonded devices: $e');
    }
    return devices;
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Select Device'),
      ),
      body: FutureBuilder<List<BluetoothDevice>>(
        future: _getBondedDevices(context),
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return Center(child: CircularProgressIndicator());
          } else if (snapshot.hasError) {
            return Center(child: Text('Error: ${snapshot.error}'));
          } else if (!snapshot.hasData || snapshot.data!.isEmpty) {
            return Center(child: Text('No bonded devices found.'));
          } else {
            return ListView.builder(
              itemCount: snapshot.data!.length,
              itemBuilder: (context, index) {
                BluetoothDevice device = snapshot.data![index];
                return ListTile(
                  title: Text(device.name ?? 'Unknown Name'),
                  subtitle: Text(device.address ?? 'Unknown Address'),
                  onTap: () {
                    Navigator.pop(context, device);
                  },
                );
              },
            );
          }
        },
      ),
    );
  }
}

class CheckScreen extends StatefulWidget {
  final BluetoothConnection connection;
  // final String userName;

  CheckScreen({required this.connection});

  @override
  _CheckScreenState createState() => _CheckScreenState();
}

class _CheckScreenState extends State<CheckScreen> {
  int? elapsedTime; // New state variable for elapsed time
  String stage = "0";
  String _messageBuffer = '';
  List<String> messages = [];

  @override
  void initState() {
    super.initState();
    _startListening();
  }

  void _startListening() {
    widget.connection.input!.listen((Uint8List data) {
      _onDataReceived(data);
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('bluetooth classic test'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            ElevatedButton(
              onPressed: () => _showSettings(context),
              child: Text('Send settings to ESP32'),
            ),
            SizedBox(height: 20),
            ElevatedButton(
              //this button does nothing
              onPressed: () {},
              child: Text('Incoming Messages:'),
            ),
            ListView.builder(
              shrinkWrap: true,
              itemCount: messages.length,
              itemBuilder: (context, index) {
                return Center(
                    child:
                        Text(messages[index])); // Wrap Text widget with Center
              },
            ),
          ],
        ),
      ),
    );
  }

  void _showSettings(BuildContext context) {
    bool isSlowMode = false;
    String? selectedSoundSet = 'sound_set_1'; // Default value

    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: Text('Settings'),
          content: StatefulBuilder(
            builder: (BuildContext context, StateSetter setState) {
              return Column(
                mainAxisSize: MainAxisSize.min,
                children: <Widget>[
                  SwitchListTile(
                    title: Text('Mode'),
                    value: isSlowMode,
                    onChanged: (bool value) {
                      setState(() {
                        isSlowMode = value;
                      });
                    },
                  ),
                  SizedBox(height: 16.0),
                  Align(
                    alignment: Alignment.centerLeft,
                    child: Text(
                      'Choose Sound',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ),
                  ListTile(
                    title: Text('Sound Set 1'),
                    leading: Radio(
                      value: 'S1',
                      groupValue: selectedSoundSet,
                      onChanged: (String? value) {
                        setState(() {
                          selectedSoundSet = value;
                        });
                      },
                    ),
                  ),
                  ListTile(
                    title: Text('Sound Set 2'),
                    leading: Radio(
                      value: 'S2',
                      groupValue: selectedSoundSet,
                      onChanged: (String? value) {
                        setState(() {
                          selectedSoundSet = value;
                        });
                      },
                    ),
                  ),
                  ListTile(
                    title: Text('Sound Set 3'),
                    leading: Radio(
                      value: 'S3',
                      groupValue: selectedSoundSet,
                      onChanged: (String? value) {
                        setState(() {
                          selectedSoundSet = value;
                        });
                      },
                    ),
                  ),
                  SizedBox(height: 16.0),
                ],
              );
            },
          ),
          actions: <Widget>[
            TextButton(
              onPressed: () {
                _sendSettings(isSlowMode, selectedSoundSet);
                Navigator.of(context).pop(); // Close the settings dialog
              },
              child: Text('Save Settings'),
            ),
          ],
        );
      },
    );
  }

  void _sendSettings(bool isSlowMode, String? selectedSoundSet) {
    String str = '';

    if (!isSlowMode) {
      str += 'M1';
    } else {
      str += 'M2';
    }
    str += ',';
    str += '$selectedSoundSet';
    str += '#';
    if (widget.connection.isConnected) {
      Uint8List data = Uint8List.fromList(utf8.encode(str));
      widget.connection.output.add(data);
      widget.connection.output.allSent.then((_) {
        print('sending: ' + str);
      });
    } else {
      print('Not connected');
    }
  }

  void _onDataReceived(Uint8List data) {
    Uint8List buffer = Uint8List(data.length);
    int bufferIndex = buffer.length;
    for (int i = 0; i < data.length; i++) {
      buffer[i] = data[i];
    }

    String dataString = String.fromCharCodes(buffer);

    int index = buffer.indexOf(35); // 35 is the # character
    if (~index != 0) {
      messages.add(_messageBuffer + dataString.substring(0, index));
      if (messages.length > 5) {
        messages.removeRange(
            0, messages.length - 5); //keep only the last 5 incoming messages
      }
      _messageBuffer =
          dataString.substring(index + 1); // Add +1 to skip the # character
      print("messages: " + messages.toString());
      setState(() {
        // Trigger an update of the scaffold to show the new messages
      });
    } else {
      _messageBuffer = _messageBuffer + dataString;
    }
  }
}
