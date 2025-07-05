import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:provider/provider.dart';

class BluetoothDeviceProvider with ChangeNotifier {
  BluetoothDevice? _connectedDevice;
  BluetoothCharacteristic? _writeCharacteristic;
  BluetoothCharacteristic? _notifyCharacteristic;
  BluetoothCharacteristic? _readCharacteristic;
  BluetoothCharacteristic? _pushNotifyCharacteristic;
  int _cocktailVersion = 0;
  int get cocktailVersion => _cocktailVersion;

  BluetoothDevice? get connectedDevice => _connectedDevice;
  BluetoothCharacteristic? get writeCharacteristic => _writeCharacteristic;
  BluetoothCharacteristic? get notifyCharacteristic => _notifyCharacteristic;
  BluetoothCharacteristic? get readCharacteristic => _readCharacteristic;
  BluetoothCharacteristic? get pushNotifyCharacteristic => _pushNotifyCharacteristic;

  void notifyCocktailsUpdated() {
    _cocktailVersion++;
    notifyListeners();
  }

  void monitorConnection() {
  _connectedDevice?.connectionState.listen((state) {
    if (state == BluetoothConnectionState.disconnected) {
      _connectedDevice = null;
      _writeCharacteristic = null;
      _notifyCharacteristic = null;
      _readCharacteristic = null;
      _pushNotifyCharacteristic = null;
      notifyListeners();
    }
  });
  }

  void setConnectedDevice(BluetoothDevice? device) {
    _connectedDevice = device;
    notifyListeners();
  }

  void setCharacteristics({
    BluetoothCharacteristic? write,
    BluetoothCharacteristic? notify,
    BluetoothCharacteristic? read,
    BluetoothCharacteristic? pushNotify,
  }) {
    _writeCharacteristic = write;
    _notifyCharacteristic = notify;
    _readCharacteristic = read;
    _pushNotifyCharacteristic = pushNotify;
    notifyListeners();
  }

  Future<List<int>?> sendRequest(String request) async {
    if (_writeCharacteristic == null) return null;
    await _writeCharacteristic!.write(utf8.encode("REQUEST $request"));
    if (_readCharacteristic == null) return null;
    return await _readCharacteristic!.read();
  }

  Future<bool> sendPost(String what, String data) async {
    if (_writeCharacteristic == null) return false;
    await _writeCharacteristic!.write(utf8.encode("POST $what $data"));
    return true;
  }

  void disconnect() {
    _connectedDevice?.disconnect();
    _connectedDevice = null;
    _writeCharacteristic = null;
    _notifyCharacteristic = null;
    _readCharacteristic = null;
    notifyListeners();
  }
}

extension SnackbarExtension on BuildContext {
  void showSnackbar(String message, {bool success = true}) {
    ScaffoldMessenger.of(this).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: success ? Colors.green : Colors.red,
      ),
    );
  }
}

final FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin = FlutterLocalNotificationsPlugin();

void initializeNotifications() async {
  const AndroidInitializationSettings initializationSettingsAndroid = AndroidInitializationSettings('@mipmap/ic_launcher'); // your app icon

  const InitializationSettings initializationSettings = InitializationSettings(
    android: initializationSettingsAndroid,
  );

  await flutterLocalNotificationsPlugin.initialize(initializationSettings);
}

void showHelloNotification() {
  const AndroidNotificationDetails androidDetails = AndroidNotificationDetails(
    'channel_id',
    'channel_name',
    importance: Importance.high,
    priority: Priority.high,
  );

  const NotificationDetails notificationDetails = NotificationDetails(android: androidDetails);

  flutterLocalNotificationsPlugin.show(
    0,
    'Cocktail Machine App', // title
    'Welcome 🤩', // body
    notificationDetails,
  );
}

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  initializeNotifications();
  showHelloNotification();
  runApp(
    ChangeNotifierProvider(
      create: (_) => BluetoothDeviceProvider(),
      child: MaterialApp(
        builder: (context, child) => SafeArea(child: child!),
        theme: ThemeData(
          appBarTheme: const AppBarTheme(
            backgroundColor: Colors.blueAccent,
            foregroundColor: Colors.white,
          ),
        ),
        home: const MainScreen(),
      ),
    ),
  );
}

class Cocktail {
  String name;
  List<int> amounts;

  Cocktail({required this.name, required this.amounts});

  bool get isComplete => amounts.reduce((a, b) => a + b) == 100;

  factory Cocktail.fromJson(Map<String, dynamic> json) {
    return Cocktail(
      name: json['name'],
      amounts: List<int>.from(json['amounts']),
    );
  }

  Map<String, dynamic> toJson() => {
        'name': name,
        'amounts': amounts,
      };
}

List<Cocktail> cocktails = List.generate(
  9,
  (index) => Cocktail(name: '[missing]', amounts: [0, 0, 0, 0]),
);

class Ingredient {
  String name;
  int amount;

  Ingredient({required this.name, required this.amount});

  factory Ingredient.fromJson(Map<String, dynamic> json) {
    return Ingredient(
      name: json['name'],
      amount: json['amount'],
    );
  }

  Map<String, dynamic> toJson() => {
        'name': name,
        'amount': amount,
      };
}

List<Ingredient> stock = List.generate(
  4,
  (index) => Ingredient(name: '[missing]', amount: 0),
);

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Cocktail Manager"),
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
              decoration: BoxDecoration(color: Colors.blueAccent),
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
                  MaterialPageRoute(builder: (context) => const BluetoothScreen()),
                ).then((_) => setState(() {}));
              },
            ),
            ListTile(
              leading: const Icon(Icons.edit),
              title: const Text('Edit Cocktails'),
              onTap: () {
                final provider = context.read<BluetoothDeviceProvider>();
                if (provider.connectedDevice == null) {
                  Navigator.pop(context); // CLOSE drawer
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('No device connected')),
                  );
                  return;
                }
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => const CocktailsListScreen()),
                ).then((_) => setState(() {}));
              },
            ),
            ListTile(
              leading: const Icon(Icons.liquor),
              title: const Text('Edit Ingredients'),
              onTap: () {
                final provider = context.read<BluetoothDeviceProvider>();
                if (provider.connectedDevice == null) {
                  Navigator.pop(context); // CLOSE drawer
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('No device connected')),
                  );
                  return;
                }
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => const IngredientsEditScreen()),
                ).then((_) => setState(() {}));
              },
            ),
            ListTile(
              leading: const Icon(Icons.cleaning_services),
              title: const Text('Clean Hoses'),
              onTap: () {
                final provider = context.read<BluetoothDeviceProvider>();
                if (provider.connectedDevice == null) {
                  Navigator.pop(context); // CLOSE drawer
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('No device connected')),
                  );
                  return;
                }
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => const CleanHoseScreen()),
                );
              },
            ),
            ListTile(
              leading: const Icon(Icons.query_stats),
              title: const Text('View Statistics'),
              onTap: () {
                final provider = context.read<BluetoothDeviceProvider>();
                if (provider.connectedDevice == null) {
                  Navigator.pop(context); // CLOSE drawer
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('No device connected')),
                  );
                  return;
                }
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => StatisticsScreen()),
                );
              },
            ),
            ListTile(
              leading: const Icon(Icons.rocket_launch),
              title: const Text('RISH'),
              onTap: () {
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => DumpScreen()),
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
                          style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                        ),
                        const SizedBox(height: 10),
                        Row(
                          children: [
                            Icon(
                              connectedDevice != null ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                              color: connectedDevice != null ? Colors.green : Colors.red,
                            ),
                            const SizedBox(width: 8),
                            Text(
                              connectedDevice != null ? "Connected to: ${connectedDevice.platformName}" : "No device connected",
                              style: TextStyle(
                                fontSize: 16,
                                color: connectedDevice != null ? Colors.green : Colors.red,
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
            Consumer<BluetoothDeviceProvider>(
              builder: (context, provider, _) {
                final _ = provider.cocktailVersion; // force rebuild
                int completeCount = cocktails.where((c) => c.isComplete).length;
                return Card(
                  child: Padding(
                    padding: const EdgeInsets.all(16.0),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        const Text(
                          'Cocktail Status',
                          style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                        ),
                        const SizedBox(height: 10),
                        Text(
                          'Complete Cocktails: $completeCount / 9',
                          style: const TextStyle(fontSize: 16),
                        ),
                        const SizedBox(height: 5),
                        LinearProgressIndicator(
                          value: completeCount / 9,
                          valueColor: AlwaysStoppedAnimation<Color>(
                            completeCount == 9 ? Colors.green : Colors.blue,
                          ),
                        ),
                      ],
                    ),
                  ),
                );
              },
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
                        MaterialPageRoute(builder: (context) => const BluetoothScreen()),
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
                      final provider = context.read<BluetoothDeviceProvider>();
                      if (provider.connectedDevice == null) {
                        ScaffoldMessenger.of(context).showSnackBar(
                          const SnackBar(content: Text('No device connected')),
                        );
                        return;
                      }
                      Navigator.push(
                        context,
                        MaterialPageRoute(builder: (context) => const CocktailsListScreen()),
                      ).then((_) => setState(() {}));
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
                final _ = provider.cocktailVersion; // force rebuild
                final connectedDevice = provider.connectedDevice;
                int completeCount = cocktails.where((c) => c.isComplete).length;
                if (connectedDevice != null && completeCount == 9) {
                  return Column(
                    children: [
                      SizedBox(
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
                      ),
                      const SizedBox(height: 12),
                      SizedBox(
                        width: double.infinity,
                        child: ElevatedButton.icon(
                          onPressed: () => _sendStock(context, provider),
                          icon: const Icon(Icons.send),
                          label: const Text('Update Stock on ESP32'),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.green,
                            padding: const EdgeInsets.symmetric(vertical: 16),
                          ),
                        ),
                      ),
                    ],
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
                      style: const TextStyle(color: Colors.orange, fontSize: 16),
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
              child: Consumer<BluetoothDeviceProvider>(
                builder: (context, provider, child) {
                  if (provider.connectedDevice == null) {
                    return Expanded(
                      child: Center(
                        child: Text(
                          'No Bluetooth device connected',
                          style: TextStyle(fontSize: 18, color: Colors.red),
                        ),
                      ),
                    );
                  }
                  final _ = provider.cocktailVersion;
                  final localCocktails = cocktails;
                  return ListView.builder(
                    itemCount: localCocktails.length,
                    itemBuilder: (context, index) {
                      final cocktail = localCocktails[index];
                      return Card(
                        child: ListTile(
                          leading: Icon(
                            cocktail.isComplete ? Icons.check_circle : Icons.radio_button_unchecked,
                            color: cocktail.isComplete ? Colors.green : Colors.grey,
                          ),
                          title: Text(cocktail.name),
                          subtitle: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: List.generate(4, (i) {
                              final name = stock[i].name;
                              final amount = cocktail.amounts[i];
                              return Text('$name: $amount ml');
                            }),
                          ),
                          trailing: Text(
                            cocktail.isComplete ? 'Complete' : 'Incomplete',
                            style: TextStyle(
                              color: cocktail.isComplete ? Colors.green : Colors.red,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                        ),
                      );
                    },
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }

  Future<void> _sendCocktailMenu(BuildContext context, BluetoothDeviceProvider provider) async {
    final device = provider.connectedDevice;
    if (device == null) {
      context.showSnackbar("No device connected", success: false);
      return;
    }

    try {
      showDialog(
        context: context,
        barrierDismissible: false,
        builder: (context) => const Center(child: CircularProgressIndicator()),
      );

      List<Map<String, dynamic>> cocktailData = cocktails.map((c) => c.toJson()).toList();
      String jsonString = jsonEncode(cocktailData);

      if (await provider.sendPost("Menu", jsonString)) {
        Navigator.of(context).pop();
        context.showSnackbar("Cocktail menu sent successfully!", success: true);
      } else {
        Navigator.of(context).pop();
        context.showSnackbar("Error sending menu", success: false);
      }
    } catch (e) {
      Navigator.of(context).pop(); // Close loading dialog
      context.showSnackbar("Error sending menu: $e", success: false);
    }
  }

  Future<void> _sendStock(BuildContext context, BluetoothDeviceProvider provider) async {
    final device = provider.connectedDevice;
    if (device == null) {
      context.showSnackbar("No device connected", success: false);
      return;
    }

    try {
      showDialog(
        context: context,
        barrierDismissible: false,
        builder: (context) => const Center(child: CircularProgressIndicator()),
      );
      List<Map<String, dynamic>> stockData = stock.map((i) => i.toJson()).toList();
      String jsonString = jsonEncode(stockData);

      if (await provider.sendPost("Stock", jsonString)) {
        Navigator.of(context).pop();
        context.showSnackbar("Stock sent successfully!", success: true);
      } else {
        Navigator.of(context).pop();
        context.showSnackbar("Error sending stock", success: false);
      }
    } catch (e) {
      Navigator.of(context).pop(); // Close loading dialog
      context.showSnackbar("Error sending stock: $e", success: false);
    }
  }
}

class BluetoothScreen extends StatefulWidget {
  const BluetoothScreen({super.key});

  @override
  State<BluetoothScreen> createState() => _BluetoothScreenState();
}

class _BluetoothScreenState extends State<BluetoothScreen> {
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
      statusMessage = allGranted ? "Permissions granted" : "Some permissions denied. BLE may not work properly.";
    });
  }

  Future<void> checkBluetoothState() async {
    try {
      BluetoothAdapterState adapterState = await FlutterBluePlus.adapterState.first;

      setState(() {
        if (adapterState == BluetoothAdapterState.on) {
          statusMessage = "Bluetooth is ready";
        } else {
          statusMessage = "Bluetooth is ${adapterState.toString().split('.').last}";
        }
      });

      if (adapterState != BluetoothAdapterState.on) {
        context.showSnackbar("Please enable Bluetooth to scan for devices", success: false);
      }
    } catch (e) {
      setState(() {
        statusMessage = "Error checking Bluetooth: $e";
      });
    }
  }

  Future<void> scanForDevices() async {
    BluetoothAdapterState adapterState = await FlutterBluePlus.adapterState.first;
    if (adapterState != BluetoothAdapterState.on) {
      context.showSnackbar("Bluetooth is not enabled", success: false);
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
          if (!devicesList.any((device) => device.remoteId == result.device.remoteId)) {
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
            statusMessage = devicesList.isEmpty ? "No devices found" : "Scan complete - ${devicesList.length} device(s) found";
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
    final provider = Provider.of<BluetoothDeviceProvider>(context, listen: false);

    setState(() {
      statusMessage = "Connecting to ${device.platformName.isNotEmpty ? device.platformName : 'Unknown'}...";
    });

    try {
      await device.connect(timeout: const Duration(seconds: 15), autoConnect: false);

      await Permission.bluetooth.request(); // Android 12+
      await Permission.bluetoothConnect.request(); // Android 12+
      await Permission.bluetoothScan.request(); // Android 12+
      await Permission.location.request(); // Sometimes still needed

      await device.requestMtu(247);

      List<BluetoothService> services = await device.discoverServices();
      BluetoothCharacteristic? writeChar;
      BluetoothCharacteristic? notifyChar;
      BluetoothCharacteristic? readChar;
      BluetoothCharacteristic? pushNotifyChar;

      // Look for the specific characteristic from your working app
      for (var service in services) {
        for (var characteristic in service.characteristics) {
          GLOBAL_RISH += "\n${(characteristic.characteristicUuid.toString())}";
          if (characteristic.characteristicUuid == Guid("beb5483e-36e1-4688-b7f5-ea07361b26a8")) {
            if (characteristic.properties.write || characteristic.properties.writeWithoutResponse) {
              writeChar = characteristic;
            }
            if (characteristic.properties.notify) {
              notifyChar = characteristic;
            }
            if (characteristic.properties.read) {
              readChar = characteristic;
            }
          }
          if (characteristic.characteristicUuid == Guid("6ba7b811-9dad-11d1-80b4-00c04fd430c8")) {
            GLOBAL_RISH = "FOUND PUSH NOTIFY";
            pushNotifyChar = characteristic;
          }
        }
      }

      // Fallback to any readable characteristic if not found
      if (readChar == null) {
        for (var service in services) {
          for (var characteristic in service.characteristics) {
            if (readChar == null && characteristic.properties.read) {
              readChar = characteristic;
            }
          }
        }
      }

      provider.setConnectedDevice(device);
      provider.setCharacteristics(write: writeChar, notify: notifyChar, read: readChar, pushNotify: pushNotifyChar);

      if (pushNotifyChar != null) {
        GLOBAL_RISH = "IN LISTENER'S IF";
        await pushNotifyChar.setNotifyValue(true);
        pushNotifyChar.lastValueStream.listen((value) {
          final msg = String.fromCharCodes(value);
          GLOBAL_RISH = msg;
          // final intAmount = int.tryParse(msg) ?? 0;
          // provider.notifyCocktailsUpdated();

          final String str = String.fromCharCodes(value).trim();
          final int x = int.parse(str);

          final String name = stock[x].name;
          final String message = "Ingredient $x ($name) is running low!";

          const AndroidNotificationDetails androidDetails = AndroidNotificationDetails(
            'channel_id',
            'channel_name',
            importance: Importance.high,
            priority: Priority.high,
          );

          const NotificationDetails notificationDetails = NotificationDetails(android: androidDetails);

          flutterLocalNotificationsPlugin.show(
            0,
            'Cocktail Machine App', // title
            message, // body
            notificationDetails,
          );
        });
      }

      setState(() {
        statusMessage = "Connected successfully!";
      });

      provider.monitorConnection();
      context.showSnackbar("Connected to ${device.platformName.isNotEmpty ? device.platformName : 'Unknown'}", success: true);

      await _loadMenu();
      await _loadStock();

      // Go back to main screen
      Navigator.of(context).pop();
    } catch (e) {
      setState(() {
        statusMessage = "Connection failed: $e";
      });
      context.showSnackbar("Connection failed: $e", success: false);
    }
  }

  Future<void> _loadMenu() async {
    final provider = context.read<BluetoothDeviceProvider>();
    final value = await provider.sendRequest("Menu");
    if (value != null) {
      final decoded = String.fromCharCodes(value);
      final data = jsonDecode(decoded) as List;
      cocktails = data.map((e) => Cocktail.fromJson(e)).toList();
      provider.notifyCocktailsUpdated();
    }
  }

  Future<void> _loadStock() async {
    final provider = context.read<BluetoothDeviceProvider>();
    final value = await provider.sendRequest("Stock");
    if (value != null) {
      final decoded = String.fromCharCodes(value);
      final data = jsonDecode(decoded) as List;
      stock = data.map((e) => Ingredient.fromJson(e)).toList();
      provider.notifyCocktailsUpdated();
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
        title: const Text("Scan for Devices"),
        actions: [
          Consumer<BluetoothDeviceProvider>(
            builder: (context, provider, child) {
              if (provider.connectedDevice != null) {
                return IconButton(
                  icon: const Icon(Icons.bluetooth_connected, color: Colors.white),
                  onPressed: () {
                    provider.disconnect();
                    context.showSnackbar("Disconnected", success: true);
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
                child: Text(
                  isScanning ? "Scanning..." : "Scan for Devices",
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
                    final isConnected = provider.connectedDevice?.remoteId == device.remoteId;

                    return Card(
                      margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
                      child: ListTile(
                        title: Text(
                          device.platformName.isNotEmpty ? device.platformName : "Unknown Device",
                          style: const TextStyle(fontWeight: FontWeight.bold),
                        ),
                        subtitle: Text(device.remoteId.toString()),
                        trailing: isConnected
                            ? const Icon(Icons.check_circle, color: Colors.green)
                            : const Icon(Icons.bluetooth, color: Colors.grey),
                        onTap: isConnected ? null : () => connectToDevice(device),
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

class CocktailsListScreen extends StatefulWidget {
  const CocktailsListScreen({super.key});

  @override
  _CocktailsListScreenState createState() => _CocktailsListScreenState();
}

class _CocktailsListScreenState extends State<CocktailsListScreen> {
  @override
  Widget build(BuildContext context) {
    int completeCount = cocktails.where((c) => c.isComplete).length;

    return Scaffold(
      appBar: AppBar(title: const Text("Edit Cocktails")),
      body: Column(
        children: [
          Container(
            width: double.infinity,
            padding: const EdgeInsets.all(16.0),
            child: Column(
              children: [
                Text(
                  'Complete Cocktails: $completeCount / 9',
                  style: const TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 8),
                LinearProgressIndicator(
                  value: completeCount / 9,
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
                  margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
                  child: ListTile(
                    leading: Icon(
                      cocktail.isComplete ? Icons.check_circle : Icons.radio_button_unchecked,
                      color: cocktail.isComplete ? Colors.green : Colors.grey,
                    ),
                    title: Text(cocktail.name),
                    subtitle: Text('Amounts: ${cocktail.amounts.join(', ')} ml (Total: ${cocktail.amounts.reduce((a, b) => a + b)} ml)'),
                    trailing: IconButton(
                      icon: const Icon(Icons.edit),
                      onPressed: () async {
                        final updated = await Navigator.push(
                          context,
                          MaterialPageRoute(
                            builder: (context) => CocktailEditScreen(cocktail: cocktail),
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

class CocktailEditScreen extends StatefulWidget {
  final Cocktail cocktail;
  const CocktailEditScreen({super.key, required this.cocktail});

  @override
  _CocktailEditScreenState createState() => _CocktailEditScreenState();
}

class _CocktailEditScreenState extends State<CocktailEditScreen> {
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
    return Scaffold(
      appBar: AppBar(title: Text("Edit $name")),
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
            Text('Total: $total ml', style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
            const SizedBox(height: 10),
            for (int i = 0; i < 4; i++)
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text('${stock[i].name}: ${amounts[i]} ml', style: const TextStyle(fontSize: 16)),
                      Row(
                        children: [
                          IconButton(
                            icon: const Icon(Icons.remove_circle, color: Colors.red),
                            onPressed: () => updateAmount(i, -10),
                          ),
                          IconButton(
                            icon: const Icon(Icons.add_circle, color: Colors.green),
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
                  Navigator.pop(context, Cocktail(name: name, amounts: amounts));
                },
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                  padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                ),
                child: const Text('Save Cocktail'),
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

class IngredientsEditScreen extends StatefulWidget {
  const IngredientsEditScreen({super.key});

  @override
  _IngredientsEditScreenState createState() => _IngredientsEditScreenState();
}

class _IngredientsEditScreenState extends State<IngredientsEditScreen> {
  late List<String> names;
  late List<int> amounts;
  bool loaded = false;

  @override
  void initState() {
    super.initState();
    _loadStock();
  }

  Future<void> _loadStock() async {
    final provider = context.read<BluetoothDeviceProvider>();
    final value = await provider.sendRequest("Stock");
    if (value != null) {
      final decoded = String.fromCharCodes(value);
      final data = jsonDecode(decoded) as List;
      stock = data.map((e) => Ingredient.fromJson(e)).toList();
      provider.notifyCocktailsUpdated();
    }

    setState(() {
      names = stock.map((ingredient) => ingredient.name).toList();
      amounts = stock.map((ingredient) => ingredient.amount).toList();
      loaded = true;
    });
  }

  void updateAmount(int index, int delta) {
    setState(() {
      int newVal = amounts[index] + delta;
      if (newVal >= 0) {
        amounts[index] = newVal;
      }
    });
  }

  void saveIngredients() {
    for (int i = 0; i < stock.length; i++) {
      stock[i] = Ingredient(name: names[i], amount: amounts[i]);
    }
    Navigator.pop(context, true);
  }

  @override
  Widget build(BuildContext context) {
    if (!loaded) {
      return const Scaffold(
        body: Center(child: CircularProgressIndicator()),
      );
    }
    return Scaffold(
      appBar: AppBar(title: const Text("Edit Ingredients")),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            const Text(
              'Ingredient Stock',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 20),
            for (int i = 0; i < 4; i++)
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(12.0),
                  child: Column(
                    children: [
                      TextField(
                        decoration: InputDecoration(
                          labelText: 'Ingredient ${i + 1} Name',
                          border: const OutlineInputBorder(),
                        ),
                        controller: TextEditingController(text: names[i]),
                        onChanged: (val) => names[i] = val,
                      ),
                      const SizedBox(height: 12),
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceBetween,
                        children: [
                          SizedBox(
                            width: 120,
                            child: TextField(
                              keyboardType: TextInputType.number,
                              decoration: const InputDecoration(
                                labelText: 'Amount (ml)',
                                border: OutlineInputBorder(),
                              ),
                              controller: TextEditingController(text: amounts[i].toString()),
                              onChanged: (val) {
                                final parsed = int.tryParse(val);
                                if (parsed != null && parsed >= 0) {
                                  amounts[i] = parsed;
                                }
                              },
                            ),
                          ),
                          Row(
                            children: [
                              IconButton(
                                icon: const Icon(Icons.remove_circle, color: Colors.red),
                                onPressed: () => updateAmount(i, -50),
                              ),
                              IconButton(
                                icon: const Icon(Icons.add_circle, color: Colors.green),
                                onPressed: () => updateAmount(i, 50),
                              ),
                            ],
                          )
                        ],
                      ),
                    ],
                  ),
                ),
              ),
            const Spacer(),
            ElevatedButton(
              onPressed: saveIngredients,
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.green,
                padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
              ),
              child: const Text(
                'Save Ingredients',
                style: TextStyle(fontSize: 16),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class CleanHoseScreen extends StatelessWidget {
  const CleanHoseScreen({super.key});

  Future<void> _sendCleanHoseCommand(BuildContext context, BluetoothDeviceProvider provider, int index) async {
    final device = provider.connectedDevice;
    if (device == null) {
      context.showSnackbar("No device connected", success: false);
      return;
    }

    try {
      String payload = index.toString();
      int displayIndex = index + 1;
      final messenger = ScaffoldMessenger.of(context);

      provider.sendPost("Clean", payload).then((success) {
        messenger.showSnackBar(SnackBar(
          content: Text(success ? "Clean command sent for hose $displayIndex" : "Failed to send clean command"),
          backgroundColor: success ? Colors.green : Colors.red,
        ));
      }).catchError((e) {
        messenger.showSnackBar(SnackBar(
          content: Text("Error sending clean command: $e"),
          backgroundColor: Colors.red,
        ));
      });
    } catch (e) {
      context.showSnackbar("Error sending clean command: $e", success: false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("Clean Ingredient Hoses")),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              "To clean a hose:\n"
              "• Replace the ingredient with water\n"
              "• Tap the button to flush\n"
              "• Remove the hose and tap again to drain",
              style: TextStyle(fontSize: 16),
            ),
            const SizedBox(height: 24),
            Expanded(
              child: ListView.builder(
                itemCount: 4,
                itemBuilder: (context, i) => Card(
                  shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                  margin: const EdgeInsets.symmetric(vertical: 8),
                  elevation: 3,
                  child: ListTile(
                    contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                    title: Text('Clean Hose for Ingredient ${i + 1}'),
                    subtitle: Text('Current: ${stock[i].name}'),
                    trailing: ElevatedButton(
                      onPressed: () {
                        final provider = context.read<BluetoothDeviceProvider>();
                        _sendCleanHoseCommand(context, provider, i);
                      },
                      child: const Text("Clean"),
                    ),
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class StatisticsScreen extends StatelessWidget {
  const StatisticsScreen({super.key});

  Future<List<String>> _readStats(BuildContext context) async {
    final provider = context.read<BluetoothDeviceProvider>();
    final value = await provider.sendRequest("Stats");

    if (value == null) return [];

    final decoded = String.fromCharCodes(value);
    final data = jsonDecode(decoded);

    final stats = [
      'Orders Completed: ${data['orders_completed']}',
      'Random Drink Orders: ${data['random_drink_orders']}',
      'Preset Drink Orders: ${data['preset_drink_orders']}',
      'Orders Timed Out: ${data['orders_timed_out']}',
      'Orders Cancelled: ${data['orders_cancelled']}',
      'Custom Drink Orders: ${data['custom_drink_orders']}',
    ];

    final List<dynamic> presetCounts = data['preset_cocktail_order_counts'];
    for (var i = 0; i < presetCounts.length; i++) {
      stats.add('${cocktails[i].name}: ${presetCounts[i]}');
    }

    return stats;
  }

  @override
  Widget build(BuildContext context) {
    GLOBAL_RISH = "STATS";
    return Scaffold(
      appBar: AppBar(title: const Text("Statistics")),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: FutureBuilder<List<String>>(
          future: _readStats(context),
          builder: (context, snapshot) {
            if (!snapshot.hasData) {
              return const Center(child: CircularProgressIndicator());
            }

            final stats = snapshot.data!;
            return ListView(
              children: [
                const Padding(
                  padding: EdgeInsets.symmetric(vertical: 8.0),
                  child: Text(
                    'General Orders',
                    style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                  ),
                ),
                _statCard(Icons.check_circle, stats[0]),
                _statCard(Icons.shuffle, stats[1]),
                _statCard(Icons.local_bar, stats[2]),
                _statCard(Icons.timer_off, stats[3]),
                _statCard(Icons.cancel, stats[4]),
                _statCard(Icons.edit, stats[5]),
                const Padding(
                  padding: EdgeInsets.symmetric(vertical: 8.0),
                  child: Text(
                    'Preset Cocktail Orders',
                    style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                  ),
                ),
                ...stats.sublist(6).map((stat) => _statCard(Icons.local_drink, stat)),
              ],
            );
          },
        ),
      ),
    );
  }

  Widget _statCard(IconData icon, String label) {
    return Card(
      elevation: 2,
      margin: const EdgeInsets.symmetric(vertical: 6),
      child: ListTile(
        leading: Icon(icon, color: Colors.blueAccent, size: 32),
        title: Text(
          label,
          style: const TextStyle(fontSize: 18, fontWeight: FontWeight.w500),
        ),
      ),
    );
  }
}

String GLOBAL_RISH = "---";

class DumpScreen extends StatelessWidget {
  const DumpScreen({super.key});

  Future<String> _readDump(BuildContext context) async {
    return GLOBAL_RISH;
    final provider = context.read<BluetoothDeviceProvider>();
    final value = await provider.sendRequest("Dump");
    return value != null ? String.fromCharCodes(value) : "-None-";
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Statistics')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: FutureBuilder<String>(
          future: _readDump(context),
          builder: (context, snapshot) {
            if (!snapshot.hasData) {
              return const Center(child: CircularProgressIndicator());
            }

            final dump = snapshot.data!;
            return SingleChildScrollView(
              child: Text(dump, style: const TextStyle(fontSize: 20)),
            );
          },
        ),
      ),
    );
  }
}
