import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:provider/provider.dart';
import 'bluetooth_listener.dart';

// Bluetooth Device Provider (from your working app)
class BluetoothDeviceProvider with ChangeNotifier {
  BluetoothDevice? _connectedDevice;
  BluetoothCharacteristic? _writeCharacteristic;
  BluetoothCharacteristic? _notifyCharacteristic;

  BluetoothDevice? get connectedDevice => _connectedDevice;
  BluetoothCharacteristic? get writeCharacteristic => _writeCharacteristic;
  BluetoothCharacteristic? get notifyCharacteristic => _notifyCharacteristic;

  void setConnectedDevice(BluetoothDevice? device) {
    _connectedDevice = device;
    notifyListeners();
  }

  void setCharacteristics(
      BluetoothCharacteristic? write, BluetoothCharacteristic? notify) {
    _writeCharacteristic = write;
    _notifyCharacteristic = notify;
    notifyListeners();
  }

  void disconnect() {
    stopListening();
    _connectedDevice?.disconnect();
    _connectedDevice = null;
    _writeCharacteristic = null;
    _notifyCharacteristic = null;
    notifyListeners();
    _receivedData.clear();
    _lastDataTime = '';
    _isListening = false;
  }

  // NEW: Data receiving functionality
  List<Map<String, dynamic>> _receivedData = [];
  bool _isListening = false;
  String _lastDataTime = '';

  List<Map<String, dynamic>> get receivedData => _receivedData;
  bool get isListening => _isListening;
  String get lastDataTime => _lastDataTime;

  // Start listening for incoming data
  void startListening() async {
    if (_notifyCharacteristic == null || _isListening) return;

    try {
      await _notifyCharacteristic!.setNotifyValue(true);
      _notifyCharacteristic!.lastValueStream.listen((data) {
        _parseIncomingData(data);
      });
      _isListening = true;
      notifyListeners();
    } catch (e) {
      // Handle error
      print('Error starting to listen: $e');
    }
  }

  // Stop listening
  void stopListening() async {
    if (_notifyCharacteristic == null || !_isListening) return;

    try {
      await _notifyCharacteristic!.setNotifyValue(false);
      _isListening = false;
      notifyListeners();
    } catch (e) {
      print('Error stopping listening: $e');
    }
  }

  // Parse incoming data from ESP32
  void _parseIncomingData(List<int> data) {
    try {
      String jsonString = utf8.decode(data);
      Map<String, dynamic> parsedData = jsonDecode(jsonString);

      // Add timestamp
      parsedData['timestamp'] = DateTime.now().toIso8601String();

      _receivedData.insert(0, parsedData); // Add to beginning

      // Keep only last 50 entries to prevent memory issues
      if (_receivedData.length > 50) {
        _receivedData = _receivedData.sublist(0, 50);
      }

      _lastDataTime = DateTime.now().toString().substring(11, 19);
      notifyListeners();
    } catch (e) {
      // Handle parsing error - maybe add error entry
      _receivedData.insert(0, {
        'error': 'Failed to parse data: $e',
        'timestamp': DateTime.now().toIso8601String(),
        'raw_data': data.toString()
      });
      notifyListeners();
    }
  }

  // Clear received data
  void clearReceivedData() {
    _receivedData.clear();
    _lastDataTime = '';
    notifyListeners();
  }
}

final GlobalKey<ScaffoldMessengerState> snackbarKey =
    GlobalKey<ScaffoldMessengerState>();

class Snackbar {
  static show(String message, {bool success = true}) {
    snackbarKey.currentState?.showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: success ? Colors.green : Colors.red,
      ),
    );
  }
}

void main() => runApp(const CocktailBluetoothApp());

class CocktailBluetoothApp extends StatelessWidget {
  const CocktailBluetoothApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (context) => BluetoothDeviceProvider(),
      child: MaterialApp(
        scaffoldMessengerKey: snackbarKey,
        builder: (context, child) => SafeArea(child: child!),
        home: const MainScreen(),
      ),
    );
  }
}

class Cocktail {
  String name;
  List<int> amounts;

  Cocktail({required this.name, required this.amounts});

  bool get isComplete => amounts.reduce((a, b) => a + b) == 100;

  Map<String, dynamic> toJson() => {
        'name': name,
        'amounts': amounts,
      };
}

// List<Cocktail> cocktails = List.generate(
//   9,
//   (index) => Cocktail(name: 'Cocktail ${index + 1}', amounts: [100, 0, 0, 0]),
// );
List<Cocktail> cocktails = [
  Cocktail(name: 'Margarita', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Mojito', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Old Fashioned', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Cosmopolitan', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Daiquiri', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Negroni', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Mai Tai', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Manhattan', amounts: [100, 0, 0, 0]),
  Cocktail(name: 'Whiskey Sour', amounts: [100, 0, 0, 0]),
];

class MainScreen extends StatelessWidget {
  const MainScreen({super.key});

  @override
  Widget build(BuildContext context) {
    int completeCount = cocktails.where((c) => c.isComplete).length;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Cocktail Manager'),
        leading: Builder(
          builder: (context) => IconButton(
            icon: const Icon(Icons.menu),
            onPressed: () => Scaffold.of(context).openDrawer(),
          ),
        ),
      ),
      drawer: Drawer(
        child: ListView(
          padding: EdgeInsets.zero,
          children: [
            const DrawerHeader(
              decoration: BoxDecoration(color: Colors.blue),
              child: Text(
                'Cocktail Manager',
                style: TextStyle(color: Colors.white, fontSize: 24),
              ),
            ),
            ListTile(
              leading: const Icon(Icons.bluetooth),
              title: const Text('Connect to Bluetooth'),
              onTap: () {
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => const ScanScreen()),
                );
              },
            ),
            ListTile(
              leading: const Icon(Icons.edit),
              title: const Text('Edit Cocktails'),
              onTap: () {
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(
                      builder: (context) => const CocktailEditScreen()),
                );
              },
            ),
            ListTile(
              leading: const Icon(Icons.query_stats),
              title: const Text('View Statistics'),
              onTap: () {
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(
                      builder: (context) => const StatisticsScreen()),
                );
              },
            ),
            ListTile(
              leading: const Icon(Icons.query_stats),
              title: const Text('RISH'),
              onTap: () {
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(
                      // builder: (context) => const BluetoothListener()),
                      builder: (context) => const StatisticsScreen()),
                );
              },
            ),
          ],
        ),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Bluetooth Status Card
            Consumer<BluetoothDeviceProvider>(
              builder: (context, provider, child) {
                final connectedDevice = provider.connectedDevice;
                return Card(
                  child: Padding(
                    padding: const EdgeInsets.all(16.0),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        const Text(
                          'Bluetooth Status',
                          style: TextStyle(
                              fontSize: 20, fontWeight: FontWeight.bold),
                        ),
                        const SizedBox(height: 10),
                        Row(
                          children: [
                            Icon(
                              connectedDevice != null
                                  ? Icons.bluetooth_connected
                                  : Icons.bluetooth_disabled,
                              color: connectedDevice != null
                                  ? Colors.green
                                  : Colors.red,
                            ),
                            const SizedBox(width: 8),
                            Text(
                              connectedDevice != null
                                  ? "Connected to: ${connectedDevice.remoteId}"
                                  : "No device connected",
                              style: TextStyle(
                                fontSize: 16,
                                color: connectedDevice != null
                                    ? Colors.green
                                    : Colors.red,
                              ),
                            ),
                          ],
                        ),
                      ],
                    ),
                  ),
                );
              },
            ),
            const SizedBox(height: 20),

            // Cocktail Status Card
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Cocktail Status',
                      style:
                          TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                    ),
                    const SizedBox(height: 10),
                    Text(
                      'Complete Cocktails: $completeCount / 9',
                      style: const TextStyle(fontSize: 16),
                    ),
                    const SizedBox(height: 5),
                    LinearProgressIndicator(
                      value: completeCount / 9,
                      backgroundColor: Colors.grey[300],
                      valueColor: AlwaysStoppedAnimation<Color>(
                        completeCount == 9 ? Colors.green : Colors.blue,
                      ),
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 20),

            // Quick Actions
            const Text(
              'Quick Actions',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 10),
            Row(
              children: [
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: () {
                      Navigator.push(
                        context,
                        MaterialPageRoute(
                            builder: (context) => const ScanScreen()),
                      );
                    },
                    icon: const Icon(Icons.bluetooth),
                    label: const Text('Bluetooth'),
                    style: ElevatedButton.styleFrom(
                      padding: const EdgeInsets.symmetric(vertical: 16),
                    ),
                  ),
                ),
                const SizedBox(width: 10),
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: () {
                      Navigator.push(
                        context,
                        MaterialPageRoute(
                            builder: (context) => const CocktailEditScreen()),
                      );
                    },
                    icon: const Icon(Icons.edit),
                    label: const Text('Edit Cocktails'),
                    style: ElevatedButton.styleFrom(
                      padding: const EdgeInsets.symmetric(vertical: 16),
                    ),
                  ),
                ),
              ],
            ),
            const SizedBox(height: 10),

            // Send Cocktail Menu Button
            Consumer<BluetoothDeviceProvider>(
              builder: (context, provider, child) {
                final connectedDevice = provider.connectedDevice;
                if (connectedDevice != null && completeCount == 9) {
                  return SizedBox(
                    width: double.infinity,
                    child: ElevatedButton.icon(
                      onPressed: () => _sendCocktailMenu(context, provider),
                      icon: const Icon(Icons.send),
                      label: const Text('Send Cocktail Menu to ESP32'),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.green,
                        padding: const EdgeInsets.symmetric(vertical: 16),
                      ),
                    ),
                  );
                } else if (connectedDevice != null && completeCount < 9) {
                  return Container(
                    width: double.infinity,
                    padding: const EdgeInsets.all(16),
                    decoration: BoxDecoration(
                      color: Colors.orange[100],
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(color: Colors.orange),
                    ),
                    child: Text(
                      'Complete all cocktails (100 ml each) to send ($completeCount/9 complete)',
                      style:
                          const TextStyle(color: Colors.orange, fontSize: 16),
                      textAlign: TextAlign.center,
                    ),
                  );
                }
                return const SizedBox.shrink();
              },
            ),
            const SizedBox(height: 20),

            // Cocktail Overview
            const Text(
              'Cocktail Overview',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 10),
            Expanded(
              child: ListView.builder(
                itemCount: cocktails.length,
                itemBuilder: (context, index) {
                  final cocktail = cocktails[index];
                  return Card(
                    child: ListTile(
                      leading: Icon(
                        cocktail.isComplete
                            ? Icons.check_circle
                            : Icons.radio_button_unchecked,
                        color: cocktail.isComplete ? Colors.green : Colors.grey,
                      ),
                      title: Text(cocktail.name),
                      subtitle: Text(
                          'Total: ${cocktail.amounts.reduce((a, b) => a + b)} ml'),
                      trailing: Text(
                        cocktail.isComplete ? 'Complete' : 'Incomplete',
                        style: TextStyle(
                          color:
                              cocktail.isComplete ? Colors.green : Colors.red,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ),
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }

  Future<void> _sendCocktailMenu(
      BuildContext context, BluetoothDeviceProvider provider) async {
    final device = provider.connectedDevice;
    if (device == null) {
      Snackbar.show("No device connected", success: false);
      return;
    }

    try {
      // Show loading dialog
      showDialog(
        context: context,
        barrierDismissible: false,
        builder: (context) => const Center(child: CircularProgressIndicator()),
      );

      List<BluetoothService> services = await device.discoverServices();
      BluetoothCharacteristic? writeCharacteristic;

      // Look for the specific characteristic from your working app
      for (var service in services) {
        for (var characteristic in service.characteristics) {
          if (characteristic.characteristicUuid ==
              Guid("beb5483e-36e1-4688-b7f5-ea07361b26a8")) {
            writeCharacteristic = characteristic;
            break;
          }
        }
        if (writeCharacteristic != null) break;
      }

      if (writeCharacteristic == null) {
        // Fallback to any writable characteristic
        for (var service in services) {
          for (var characteristic in service.characteristics) {
            if (characteristic.properties.write ||
                characteristic.properties.writeWithoutResponse) {
              writeCharacteristic = characteristic;
              break;
            }
          }
          if (writeCharacteristic != null) break;
        }
      }

      if (writeCharacteristic != null) {
        // Prepare cocktail data
        List<Map<String, dynamic>> cocktailData =
            cocktails.map((c) => c.toJson()).toList();
        String jsonString = jsonEncode(cocktailData);
        List<int> bytes = utf8.encode(jsonString);

        // Send data
        if (writeCharacteristic.properties.write) {
          await writeCharacteristic.write(bytes, withoutResponse: false);
        } else {
          await writeCharacteristic.write(bytes, withoutResponse: true);
        }

        Navigator.of(context).pop(); // Close loading dialog
        Snackbar.show("Cocktail menu sent successfully!", success: true);
      } else {
        Navigator.of(context).pop(); // Close loading dialog
        Snackbar.show("No writable characteristic found", success: false);
      }
    } catch (e) {
      Navigator.of(context).pop(); // Close loading dialog
      Snackbar.show("Error sending data: $e", success: false);
    }
  }
}

// Scan Screen (from your working app)
class ScanScreen extends StatefulWidget {
  const ScanScreen({super.key});

  @override
  State<ScanScreen> createState() => _ScanScreenState();
}

class _ScanScreenState extends State<ScanScreen> {
  List<BluetoothDevice> devicesList = [];
  bool isScanning = false;
  String statusMessage = "";

  @override
  void initState() {
    super.initState();
    initializeBluetooth();
  }

  Future<void> initializeBluetooth() async {
    await requestPermissions();
    await checkBluetoothState();
  }

  Future<void> requestPermissions() async {
    setState(() {
      statusMessage = "Requesting permissions...";
    });

    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.bluetoothAdvertise,
      Permission.locationWhenInUse,
    ].request();

    bool allGranted = statuses.values.every((status) => status.isGranted);

    setState(() {
      statusMessage = allGranted
          ? "Permissions granted"
          : "Some permissions denied. BLE may not work properly.";
    });
  }

  Future<void> checkBluetoothState() async {
    try {
      BluetoothAdapterState adapterState =
          await FlutterBluePlus.adapterState.first;

      setState(() {
        if (adapterState == BluetoothAdapterState.on) {
          statusMessage = "Bluetooth is ready";
        } else {
          statusMessage =
              "Bluetooth is ${adapterState.toString().split('.').last}";
        }
      });

      if (adapterState != BluetoothAdapterState.on) {
        Snackbar.show("Please enable Bluetooth to scan for devices",
            success: false);
      }
    } catch (e) {
      setState(() {
        statusMessage = "Error checking Bluetooth: $e";
      });
    }
  }

  Future<void> scanForDevices() async {
    BluetoothAdapterState adapterState =
        await FlutterBluePlus.adapterState.first;
    if (adapterState != BluetoothAdapterState.on) {
      Snackbar.show("Bluetooth is not enabled", success: false);
      return;
    }

    if (await FlutterBluePlus.isScanning.first) {
      await FlutterBluePlus.stopScan();
    }

    setState(() {
      devicesList.clear();
      isScanning = true;
      statusMessage = "Scanning for devices...";
    });

    try {
      await FlutterBluePlus.startScan(
        timeout: const Duration(seconds: 10),
        androidUsesFineLocation: true,
      );

      FlutterBluePlus.scanResults.listen((results) {
        for (ScanResult result in results) {
          if (!devicesList
              .any((device) => device.remoteId == result.device.remoteId)) {
            setState(() {
              devicesList.add(result.device);
              statusMessage = "Found ${devicesList.length} device(s)";
            });
          }
        }
      });

      FlutterBluePlus.isScanning.listen((scanning) {
        if (!scanning && isScanning) {
          setState(() {
            isScanning = false;
            statusMessage = devicesList.isEmpty
                ? "No devices found"
                : "Scan complete - ${devicesList.length} device(s) found";
          });
        }
      });
    } catch (e) {
      setState(() {
        isScanning = false;
        statusMessage = "Scan error: $e";
      });
    }
  }

  Future<void> connectToDevice(BluetoothDevice device) async {
    final provider =
        Provider.of<BluetoothDeviceProvider>(context, listen: false);

    setState(() {
      statusMessage =
          "Connecting to ${device.platformName.isNotEmpty ? device.platformName : 'Unknown'}...";
    });

    try {
      await device.connect(timeout: const Duration(seconds: 15));

      List<BluetoothService> services = await device.discoverServices();
      BluetoothCharacteristic? writeChar;
      BluetoothCharacteristic? notifyChar;

      // Look for the specific characteristic from your working app
      for (var service in services) {
        for (var characteristic in service.characteristics) {
          if (characteristic.characteristicUuid ==
              Guid("beb5483e-36e1-4688-b7f5-ea07361b26a8")) {
            if (characteristic.properties.write ||
                characteristic.properties.writeWithoutResponse) {
              writeChar = characteristic;
            }
            if (characteristic.properties.notify) {
              notifyChar = characteristic;
            }
          }
        }
      }

      // Fallback to any writable/notify characteristics
      if (writeChar == null || notifyChar == null) {
        for (var service in services) {
          for (var characteristic in service.characteristics) {
            if (writeChar == null &&
                (characteristic.properties.write ||
                    characteristic.properties.writeWithoutResponse)) {
              writeChar = characteristic;
            }
            if (notifyChar == null && characteristic.properties.notify) {
              notifyChar = characteristic;
            }
          }
        }
      }

      provider.setConnectedDevice(device);
      provider.setCharacteristics(writeChar, notifyChar);

      setState(() {
        statusMessage = "Connected successfully!";
      });

      Snackbar.show(
          "Connected to ${device.platformName.isNotEmpty ? device.platformName : 'Unknown'}",
          success: true);

      // Go back to main screen
      Navigator.of(context).pop();
    } catch (e) {
      setState(() {
        statusMessage = "Connection failed: $e";
      });
      Snackbar.show("Connection failed: $e", success: false);
    }
  }

  @override
  void dispose() {
    FlutterBluePlus.stopScan();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Colors.teal,
        title: const Text("Scan for Devices",
            style: TextStyle(color: Colors.white)),
        actions: [
          Consumer<BluetoothDeviceProvider>(
            builder: (context, provider, child) {
              if (provider.connectedDevice != null) {
                return IconButton(
                  icon: const Icon(Icons.bluetooth_connected,
                      color: Colors.white),
                  onPressed: () {
                    provider.disconnect();
                    Snackbar.show("Disconnected", success: true);
                  },
                );
              }
              return const SizedBox.shrink();
            },
          ),
        ],
      ),
      body: Column(
        children: [
          // Status bar
          Container(
            width: double.infinity,
            padding: const EdgeInsets.all(8.0),
            color: Colors.grey[200],
            child: Text(
              statusMessage,
              style: const TextStyle(fontSize: 12),
              textAlign: TextAlign.center,
            ),
          ),

          // Control buttons
          Padding(
            padding: const EdgeInsets.all(16.0),
            child: SizedBox(
              width: double.infinity,
              child: ElevatedButton(
                onPressed: isScanning ? null : scanForDevices,
                style: ElevatedButton.styleFrom(backgroundColor: Colors.blue),
                child: Text(
                  isScanning ? "Scanning..." : "Scan for Devices",
                  style: const TextStyle(color: Colors.white),
                ),
              ),
            ),
          ),

          // Device list
          Expanded(
            child: ListView.builder(
              itemCount: devicesList.length,
              itemBuilder: (context, index) {
                final device = devicesList[index];
                return Consumer<BluetoothDeviceProvider>(
                  builder: (context, provider, child) {
                    final isConnected =
                        provider.connectedDevice?.remoteId == device.remoteId;

                    return Card(
                      margin: const EdgeInsets.symmetric(
                          horizontal: 16, vertical: 4),
                      child: ListTile(
                        title: Text(
                          device.platformName.isNotEmpty
                              ? device.platformName
                              : "Unknown Device",
                          style: const TextStyle(fontWeight: FontWeight.bold),
                        ),
                        subtitle: Text(device.remoteId.toString()),
                        trailing: isConnected
                            ? const Icon(Icons.check_circle,
                                color: Colors.green)
                            : const Icon(Icons.bluetooth, color: Colors.grey),
                        onTap:
                            isConnected ? null : () => connectToDevice(device),
                        tileColor: isConnected ? Colors.green[100] : null,
                      ),
                    );
                  },
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}

// Keep your existing CocktailEditScreen and EditCocktailScreen classes unchanged
class CocktailEditScreen extends StatefulWidget {
  const CocktailEditScreen({super.key});

  @override
  _CocktailEditScreenState createState() => _CocktailEditScreenState();
}

class _CocktailEditScreenState extends State<CocktailEditScreen> {
  @override
  Widget build(BuildContext context) {
    int completeCount = cocktails.where((c) => c.isComplete).length;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Edit Cocktails'),
      ),
      body: Column(
        children: [
          Container(
            width: double.infinity,
            padding: const EdgeInsets.all(16.0),
            color: Colors.grey[100],
            child: Column(
              children: [
                Text(
                  'Complete Cocktails: $completeCount / 9',
                  style: const TextStyle(
                      fontSize: 16, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 8),
                LinearProgressIndicator(
                  value: completeCount / 9,
                  backgroundColor: Colors.grey[300],
                  valueColor: AlwaysStoppedAnimation<Color>(
                    completeCount == 9 ? Colors.green : Colors.blue,
                  ),
                ),
              ],
            ),
          ),
          Expanded(
            child: ListView.builder(
              itemCount: cocktails.length,
              itemBuilder: (context, index) {
                final cocktail = cocktails[index];
                return Card(
                  margin:
                      const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
                  child: ListTile(
                    leading: Icon(
                      cocktail.isComplete
                          ? Icons.check_circle
                          : Icons.radio_button_unchecked,
                      color: cocktail.isComplete ? Colors.green : Colors.grey,
                    ),
                    title: Text(cocktail.name),
                    subtitle: Text(
                        'Amounts: ${cocktail.amounts.join(', ')} ml (Total: ${cocktail.amounts.reduce((a, b) => a + b)} ml)'),
                    trailing: IconButton(
                      icon: const Icon(Icons.edit),
                      onPressed: () async {
                        final updated = await Navigator.push(
                          context,
                          MaterialPageRoute(
                            builder: (context) =>
                                EditCocktailScreen(cocktail: cocktail),
                          ),
                        );
                        if (updated != null) {
                          setState(() {
                            cocktails[index] = updated;
                          });
                        }
                      },
                    ),
                  ),
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}

class EditCocktailScreen extends StatefulWidget {
  final Cocktail cocktail;
  const EditCocktailScreen({super.key, required this.cocktail});

  @override
  _EditCocktailScreenState createState() => _EditCocktailScreenState();
}

class _EditCocktailScreenState extends State<EditCocktailScreen> {
  late String name;
  late List<int> amounts;

  @override
  void initState() {
    super.initState();
    name = widget.cocktail.name;
    amounts = List.from(widget.cocktail.amounts);
  }

  int get total => amounts.reduce((a, b) => a + b);

  void updateAmount(int index, int delta) {
    setState(() {
      int newVal = amounts[index] + delta;
      if (newVal >= 0 && total + delta <= 100) {
        amounts[index] = newVal;
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    List<String> cosmoIngredients = [
      'Vodka',
      'Triple Sec',
      'Cranberry Juice',
      'Lime Juice',
    ];

    return Scaffold(
      appBar: AppBar(title: Text('Edit $name')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            TextField(
              decoration: const InputDecoration(labelText: 'Cocktail Name'),
              controller: TextEditingController(text: name),
              onChanged: (val) => name = val,
            ),
            const SizedBox(height: 20),
            Text('Total: $total ml',
                style:
                    const TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
            const SizedBox(height: 10),
            for (int i = 0; i < 4; i++)
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text('${cosmoIngredients[i]}: ${amounts[i]} ml',
                          style: const TextStyle(fontSize: 16)),
                      Row(
                        children: [
                          IconButton(
                            icon: const Icon(Icons.remove_circle,
                                color: Colors.red),
                            onPressed: () => updateAmount(i, -10),
                          ),
                          IconButton(
                            icon: const Icon(Icons.add_circle,
                                color: Colors.green),
                            onPressed: () => updateAmount(i, 10),
                          ),
                        ],
                      )
                    ],
                  ),
                ),
              ),
            const Spacer(),
            if (total == 100)
              ElevatedButton(
                onPressed: () {
                  Navigator.pop(
                      context, Cocktail(name: name, amounts: amounts));
                },
                child: const Text('Save Cocktail'),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                  padding:
                      const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                ),
              )
            else
              const Text(
                'Total must be exactly 100ml to save',
                style: TextStyle(color: Colors.red, fontSize: 16),
              )
          ],
        ),
      ),
    );
  }
}

class StatisticsScreen extends StatefulWidget {
  const StatisticsScreen({super.key});

  @override
  _StatisticsScreenState createState() => _StatisticsScreenState();
}

class _StatisticsScreenState extends State<StatisticsScreen> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Statistics'),
      ),
      body: const Padding(
        padding: EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Stat 1 = 10', style: TextStyle(fontSize: 24)),
            SizedBox(height: 10),
            Text('Stat 2 = 25', style: TextStyle(fontSize: 24)),
            SizedBox(height: 10),
            Text('Stat 3 = 7', style: TextStyle(fontSize: 24)),
            SizedBox(height: 10),
            Text('Stat 4 = 42', style: TextStyle(fontSize: 24)),
            SizedBox(height: 10),
            Text('Stat 5 = 3', style: TextStyle(fontSize: 24)),
          ],
        ),
      ),
    );
  }

  @override
  void initState() {
    super.initState();
    // Start listening when screen opens
    WidgetsBinding.instance.addPostFrameCallback((_) {
      final provider =
          Provider.of<BluetoothDeviceProvider>(context, listen: false);
      if (provider.connectedDevice != null) {
        provider.startListening();
      }
    });
  }
}
