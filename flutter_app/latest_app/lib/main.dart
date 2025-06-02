import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() => runApp(const CocktailBluetoothApp());

class CocktailBluetoothApp extends StatelessWidget {
  const CocktailBluetoothApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      builder: (context, child) => SafeArea(child: child!),
      home: const MainScreen(),
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

class CocktailManager {
  static List<Cocktail> cocktails = List.generate(
    9,
    (index) => Cocktail(name: 'Cocktail ${index + 1}', amounts: [100, 0, 0, 0]),
  );
}

class MainScreen extends StatelessWidget {
  const MainScreen({super.key});

  @override
  Widget build(BuildContext context) {
    int completeCount =
        CocktailManager.cocktails.where((c) => c.isComplete).length;

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
              decoration: BoxDecoration(
                color: Colors.blue,
              ),
              child: Text(
                'Cocktail Manager',
                style: TextStyle(
                  color: Colors.white,
                  fontSize: 24,
                ),
              ),
            ),
            ListTile(
              leading: const Icon(Icons.bluetooth),
              title: const Text('Connect to Bluetooth'),
              onTap: () {
                Navigator.pop(context);
                Navigator.push(
                  context,
                  MaterialPageRoute(
                      builder: (context) => const BluetoothScreen()),
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
          ],
        ),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
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
                            builder: (context) => const BluetoothScreen()),
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
            const SizedBox(height: 20),
            const Text(
              'Cocktail Overview',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 10),
            Expanded(
              child: ListView.builder(
                itemCount: CocktailManager.cocktails.length,
                itemBuilder: (context, index) {
                  final cocktail = CocktailManager.cocktails[index];
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
                        'Total: ${cocktail.amounts.reduce((a, b) => a + b)} ml',
                      ),
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
}

class BluetoothScreen extends StatefulWidget {
  const BluetoothScreen({super.key});

  @override
  _BluetoothScreenState createState() => _BluetoothScreenState();
}

class _BluetoothScreenState extends State<BluetoothScreen> {
  List<BluetoothDevice> devicesList = [];
  BluetoothDevice? connectedDevice;
  BluetoothCharacteristic? writeCharacteristic;
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

    if (!allGranted) {
      print("Permission statuses: $statuses");
    }
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
        ScaffoldMessenger.of(context).showSnackBar(const SnackBar(
            content: Text('Please enable Bluetooth to scan for devices')));
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
      ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('Bluetooth is not enabled')));
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
      print("Scan error: $e");
    }
  }

  Future<void> connectToDevice(BluetoothDevice device) async {
    setState(() {
      statusMessage =
          "Connecting to ${device.platformName.isNotEmpty ? device.platformName : 'Unknown'}...";
    });

    try {
      await device.connect(timeout: const Duration(seconds: 15));

      setState(() {
        connectedDevice = device;
        statusMessage =
            "Connected to ${device.platformName.isNotEmpty ? device.platformName : 'Unknown'}";
      });

      List<BluetoothService> services = await device.discoverServices();

      BluetoothCharacteristic? writeChar;
      BluetoothCharacteristic? writeNoResponseChar;

      for (var service in services) {
        print("Service UUID: ${service.uuid}");
        for (var characteristic in service.characteristics) {
          print("  Characteristic UUID: ${characteristic.uuid}");
          print(
              "  Properties: write=${characteristic.properties.write}, writeWithoutResponse=${characteristic.properties.writeWithoutResponse}");

          if (characteristic.properties.write) {
            writeChar = characteristic;
          } else if (characteristic.properties.writeWithoutResponse) {
            writeNoResponseChar = characteristic;
          }
        }
      }

      writeCharacteristic = writeChar ?? writeNoResponseChar;

      if (writeCharacteristic != null) {
        setState(() {
          statusMessage =
              "Ready to send data (${writeCharacteristic!.properties.write ? 'WRITE' : 'WRITE_NO_RESPONSE'})";
        });
      } else {
        setState(() {
          statusMessage = "No writable characteristic found";
        });
      }
    } catch (e) {
      setState(() {
        connectedDevice = null;
        statusMessage = "Connection failed: $e";
      });
      print("Connection error: $e");
    }
  }

  Future<void> disconnectDevice() async {
    if (connectedDevice != null) {
      try {
        await connectedDevice!.disconnect();
        setState(() {
          connectedDevice = null;
          writeCharacteristic = null;
          statusMessage = "Disconnected";
        });
      } catch (e) {
        print("Disconnect error: $e");
      }
    }
  }

  void sendCocktailMenu() async {
    if (writeCharacteristic != null) {
      setState(() {
        statusMessage = "Sending cocktail menu...";
      });

      try {
        List<Map<String, dynamic>> cocktailData =
            CocktailManager.cocktails.map((c) => c.toJson()).toList();
        String jsonString = jsonEncode(cocktailData);

        print("=== SENDING COCKTAIL DATA ===");
        print("JSON String: $jsonString");
        print("=== INDIVIDUAL COCKTAILS ===");
        for (int i = 0; i < CocktailManager.cocktails.length; i++) {
          print("Cocktail ${i + 1}: ${CocktailManager.cocktails[i].name}");
          print("  Amounts only: ${CocktailManager.cocktails[i].amounts}");
          print("  Complete: ${CocktailManager.cocktails[i].isComplete}");
          print("  JSON: ${jsonEncode(CocktailManager.cocktails[i].toJson())}");
        }
        print("=============================");

        List<int> bytes = utf8.encode(jsonString);
        print("Sending ${bytes.length} bytes");

        int mtu = await connectedDevice!.mtu.first;
        int maxChunkSize = mtu - 3;

        if (bytes.length <= maxChunkSize) {
          if (writeCharacteristic!.properties.write) {
            await writeCharacteristic!.write(bytes, withoutResponse: false);
          } else if (writeCharacteristic!.properties.writeWithoutResponse) {
            await writeCharacteristic!.write(bytes, withoutResponse: true);
          }
        } else {
          print(
              "Data too large (${bytes.length} bytes), splitting into chunks of $maxChunkSize bytes");
          for (int i = 0; i < bytes.length; i += maxChunkSize) {
            int end = (i + maxChunkSize < bytes.length)
                ? i + maxChunkSize
                : bytes.length;
            List<int> chunk = bytes.sublist(i, end);

            print(
                "Sending chunk ${(i / maxChunkSize).floor() + 1}: ${chunk.length} bytes");

            if (writeCharacteristic!.properties.write) {
              await writeCharacteristic!.write(chunk, withoutResponse: false);
            } else if (writeCharacteristic!.properties.writeWithoutResponse) {
              await writeCharacteristic!.write(chunk, withoutResponse: true);
            }

            await Future.delayed(const Duration(milliseconds: 50));
          }
        }

        setState(() {
          statusMessage = "Cocktail menu sent successfully!";
        });

        ScaffoldMessenger.of(context)
            .showSnackBar(const SnackBar(content: Text("Cocktail menu sent!")));
      } catch (e) {
        setState(() {
          statusMessage = "Error sending data: $e";
        });
        print("Send error details: $e");
        ScaffoldMessenger.of(context)
            .showSnackBar(SnackBar(content: Text("Error sending data: $e")));
      }
    }
  }

  @override
  void dispose() {
    disconnectDevice();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    int completeCount =
        CocktailManager.cocktails.where((c) => c.isComplete).length;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Bluetooth Connection'),
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
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  onPressed: isScanning ? null : scanForDevices,
                  child: Text(isScanning ? "Scanning..." : "Scan BLE Devices"),
                ),
                if (connectedDevice != null)
                  ElevatedButton(
                    onPressed: disconnectDevice,
                    child: const Text("Disconnect"),
                    style:
                        ElevatedButton.styleFrom(backgroundColor: Colors.red),
                  ),
              ],
            ),
          ),

          // Device list
          Expanded(
            child: ListView.builder(
              itemCount: devicesList.length,
              itemBuilder: (context, index) {
                final device = devicesList[index];
                final isConnected =
                    connectedDevice?.remoteId == device.remoteId;

                return ListTile(
                  title: Text(device.platformName.isNotEmpty
                      ? device.platformName
                      : "Unknown device"),
                  subtitle: Text(device.remoteId.toString()),
                  trailing: isConnected
                      ? const Icon(Icons.check_circle, color: Colors.green)
                      : null,
                  onTap: isConnected ? null : () => connectToDevice(device),
                  tileColor: isConnected ? Colors.green[100] : null,
                );
              },
            ),
          ),

          // Send button
          if (connectedDevice != null && completeCount == 9)
            Padding(
              padding: const EdgeInsets.all(16.0),
              child: ElevatedButton(
                onPressed: sendCocktailMenu,
                child: const Text("Send Cocktail Menu"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                  padding:
                      const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                ),
              ),
            )
          else if (connectedDevice != null && completeCount < 9)
            Padding(
              padding: const EdgeInsets.all(16.0),
              child: Text(
                'Complete all cocktails (100 ml each) to send ($completeCount/9 complete)',
                style: const TextStyle(color: Colors.red),
                textAlign: TextAlign.center,
              ),
            ),
        ],
      ),
    );
  }
}

class CocktailEditScreen extends StatefulWidget {
  const CocktailEditScreen({super.key});

  @override
  _CocktailEditScreenState createState() => _CocktailEditScreenState();
}

class _CocktailEditScreenState extends State<CocktailEditScreen> {
  @override
  Widget build(BuildContext context) {
    int completeCount =
        CocktailManager.cocktails.where((c) => c.isComplete).length;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Edit Cocktails'),
      ),
      body: Column(
        children: [
          // Progress indicator
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

          // Cocktail list
          Expanded(
            child: ListView.builder(
              itemCount: CocktailManager.cocktails.length,
              itemBuilder: (context, index) {
                final cocktail = CocktailManager.cocktails[index];
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
                            CocktailManager.cocktails[index] = updated;
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
                      Text('Drink ${i + 1}: ${amounts[i]} ml',
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
