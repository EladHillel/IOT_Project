import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() => runApp(CocktailBluetoothApp());

class CocktailBluetoothApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: CocktailBluetoothScreen(),
    );
  }
}

class Cocktail {
  String name;
  Map<String, int> ingredients;

  Cocktail({required this.name, required this.ingredients});

  bool get isComplete => ingredients.values.reduce((a, b) => a + b) == 100;

  Map<String, dynamic> toJson() => {
        'name': name,
        'amounts': ingredients.values.toList(), // Only send name and amounts
      };
}

class CocktailBluetoothScreen extends StatefulWidget {
  @override
  _CocktailBluetoothScreenState createState() =>
      _CocktailBluetoothScreenState();
}

class _CocktailBluetoothScreenState extends State<CocktailBluetoothScreen> {
  List<Cocktail> cocktails = List.generate(
      9,
      (index) => Cocktail(name: 'Cocktail ${index + 1}', ingredients: {
            'Drink 1': 0,
            'Drink 2': 0,
            'Drink 3': 0,
            'Drink 4': 0,
          }));

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
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(
            content: Text('Please enable Bluetooth to scan for devices')));
      }
    } catch (e) {
      setState(() {
        statusMessage = "Error checking Bluetooth: $e";
      });
    }
  }

  Future<void> scanForDevices() async {
    // Check if Bluetooth is available and on
    BluetoothAdapterState adapterState =
        await FlutterBluePlus.adapterState.first;
    if (adapterState != BluetoothAdapterState.on) {
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text('Bluetooth is not enabled')));
      return;
    }

    // Stop any ongoing scan
    if (await FlutterBluePlus.isScanning.first) {
      await FlutterBluePlus.stopScan();
    }

    setState(() {
      devicesList.clear();
      isScanning = true;
      statusMessage = "Scanning for devices...";
    });

    try {
      // Start scanning
      await FlutterBluePlus.startScan(
        timeout: Duration(seconds: 10),
        androidUsesFineLocation: true,
      );

      // Listen to scan results
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

      // Listen for scan completion
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
      await device.connect(timeout: Duration(seconds: 15));

      setState(() {
        connectedDevice = device;
        statusMessage =
            "Connected to ${device.platformName.isNotEmpty ? device.platformName : 'Unknown'}";
      });

      // Discover services
      List<BluetoothService> services = await device.discoverServices();

      // Find writable characteristic - prioritize WRITE over WRITE_WITHOUT_RESPONSE
      BluetoothCharacteristic? writeChar;
      BluetoothCharacteristic? writeNoResponseChar;
      
      for (var service in services) {
        print("Service UUID: ${service.uuid}");
        for (var characteristic in service.characteristics) {
          print("  Characteristic UUID: ${characteristic.uuid}");
          print("  Properties: write=${characteristic.properties.write}, writeWithoutResponse=${characteristic.properties.writeWithoutResponse}");
          
          if (characteristic.properties.write) {
            writeChar = characteristic;
          } else if (characteristic.properties.writeWithoutResponse) {
            writeNoResponseChar = characteristic;
          }
        }
      }
      
      // Prefer WRITE over WRITE_WITHOUT_RESPONSE
      writeCharacteristic = writeChar ?? writeNoResponseChar;

      if (writeCharacteristic != null) {
        setState(() {
          statusMessage = "Ready to send data (${writeCharacteristic!.properties.write ? 'WRITE' : 'WRITE_NO_RESPONSE'})";
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
        // Create the JSON data with only name and amounts
        List<Map<String, dynamic>> cocktailData = cocktails.map((c) => c.toJson()).toList();
        String jsonString = jsonEncode(cocktailData);
        
        // Debug: Print what we're sending
        print("=== SENDING COCKTAIL DATA ===");
        print("JSON String: $jsonString");
        print("=== INDIVIDUAL COCKTAILS ===");
        for (int i = 0; i < cocktails.length; i++) {
          print("Cocktail ${i + 1}: ${cocktails[i].name}");
          print("  Amounts only: ${cocktails[i].ingredients.values.toList()}");
          print("  Complete: ${cocktails[i].isComplete}");
          print("  JSON: ${jsonEncode(cocktails[i].toJson())}");
        }
        print("=============================");
        
        List<int> bytes = utf8.encode(jsonString);
        print("Sending ${bytes.length} bytes");

        // Check MTU and split data if necessary
        int mtu = await connectedDevice!.mtu.first;
        int maxChunkSize = mtu - 3; // Reserve 3 bytes for BLE header
        
        if (bytes.length <= maxChunkSize) {
          // Send in one chunk
          if (writeCharacteristic!.properties.write) {
            await writeCharacteristic!.write(bytes, withoutResponse: false);
          } else if (writeCharacteristic!.properties.writeWithoutResponse) {
            await writeCharacteristic!.write(bytes, withoutResponse: true);
          }
        } else {
          // Split into chunks
          print("Data too large (${bytes.length} bytes), splitting into chunks of $maxChunkSize bytes");
          for (int i = 0; i < bytes.length; i += maxChunkSize) {
            int end = (i + maxChunkSize < bytes.length) ? i + maxChunkSize : bytes.length;
            List<int> chunk = bytes.sublist(i, end);
            
            print("Sending chunk ${(i / maxChunkSize).floor() + 1}: ${chunk.length} bytes");
            
            if (writeCharacteristic!.properties.write) {
              await writeCharacteristic!.write(chunk, withoutResponse: false);
            } else if (writeCharacteristic!.properties.writeWithoutResponse) {
              await writeCharacteristic!.write(chunk, withoutResponse: true);
            }
            
            // Small delay between chunks
            await Future.delayed(Duration(milliseconds: 50));
          }
        }

        setState(() {
          statusMessage = "Cocktail menu sent successfully!";
        });

        ScaffoldMessenger.of(context)
            .showSnackBar(SnackBar(content: Text("Cocktail menu sent!")));
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
    int completeCount = cocktails.where((c) => c.isComplete).length;

    return Scaffold(
      appBar: AppBar(title: Text('Cocktail & Bluetooth')),
      body: Column(
        children: [
          // Status bar
          Container(
            width: double.infinity,
            padding: EdgeInsets.all(8.0),
            color: Colors.grey[200],
            child: Text(
              statusMessage,
              style: TextStyle(fontSize: 12),
              textAlign: TextAlign.center,
            ),
          ),

          // Cocktails section
          Expanded(
            flex: 3,
            child: ListView(
              children: [
                for (int i = 0; i < cocktails.length; i++)
                  ListTile(
                    title: Text(
                        '${cocktails[i].isComplete ? '✅' : '❌'} - ${cocktails[i].name}'),
                    subtitle: Text(
                        'Ingredients: ${cocktails[i].ingredients.values.join(', ')} ml (Total: ${cocktails[i].ingredients.values.reduce((a, b) => a + b)} ml)'
                    ),
                    trailing: IconButton(
                      icon: Icon(Icons.edit),
                      onPressed: () async {
                        final updated = await Navigator.push(
                          context,
                          MaterialPageRoute(
                            builder: (context) =>
                                EditCocktailScreen(cocktail: cocktails[i]),
                          ),
                        );
                        if (updated != null) {
                          setState(() {
                            cocktails[i] = updated;
                          });
                        }
                      },
                    ),
                  ),
              ],
            ),
          ),

          Divider(),

          // Bluetooth section
          Expanded(
            flex: 2,
            child: Column(
              children: [
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    ElevatedButton(
                      onPressed: isScanning ? null : scanForDevices,
                      child:
                          Text(isScanning ? "Scanning..." : "Scan BLE Devices"),
                    ),
                    if (connectedDevice != null)
                      ElevatedButton(
                        onPressed: disconnectDevice,
                        child: Text("Disconnect"),
                        style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.red),
                      ),
                  ],
                ),
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
                            ? Icon(Icons.check_circle, color: Colors.green)
                            : null,
                        onTap:
                            isConnected ? null : () => connectToDevice(device),
                        tileColor: isConnected ? Colors.green[100] : null,
                      );
                    },
                  ),
                ),
                if (connectedDevice != null && completeCount == 9)
                  Padding(
                    padding: const EdgeInsets.all(8.0),
                    child: ElevatedButton(
                      onPressed: sendCocktailMenu,
                      child: Text("Send Cocktail Menu"),
                    ),
                  )
                else if (connectedDevice != null && completeCount < 9)
                  Padding(
                    padding: const EdgeInsets.all(8.0),
                    child: Text(
                      'Complete all cocktails (100 ml each) to send',
                      style: TextStyle(color: Colors.red),
                    ),
                  ),
              ],
            ),
          )
        ],
      ),
    );
  }
}

class EditCocktailScreen extends StatefulWidget {
  final Cocktail cocktail;
  EditCocktailScreen({required this.cocktail});

  @override
  _EditCocktailScreenState createState() => _EditCocktailScreenState();
}

class _EditCocktailScreenState extends State<EditCocktailScreen> {
  late String name;
  late Map<String, int> ingredients;

  @override
  void initState() {
    super.initState();
    name = widget.cocktail.name;
    ingredients = Map.from(widget.cocktail.ingredients);
  }

  int get total => ingredients.values.reduce((a, b) => a + b);

  void updateIngredient(String key, int delta) {
    setState(() {
      int newVal = ingredients[key]! + delta;
      if (newVal >= 0 && total + delta <= 100) {
        ingredients[key] = newVal;
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
              decoration: InputDecoration(labelText: 'Cocktail Name'),
              controller: TextEditingController(text: name),
              onChanged: (val) => name = val,
            ),
            SizedBox(height: 20),
            Text('Total: $total ml', style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
            SizedBox(height: 10),
            for (String drink in ingredients.keys)
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text('$drink: ${ingredients[drink]} ml', style: TextStyle(fontSize: 16)),
                      Row(
                        children: [
                          IconButton(
                            icon: Icon(Icons.remove_circle, color: Colors.red),
                            onPressed: () => updateIngredient(drink, -10),
                          ),
                          IconButton(
                            icon: Icon(Icons.add_circle, color: Colors.green),
                            onPressed: () => updateIngredient(drink, 10),
                          ),
                        ],
                      )
                    ],
                  ),
                ),
              ),
            Spacer(),
            if (total == 100)
              ElevatedButton(
                onPressed: () {
                  Navigator.pop(
                      context, Cocktail(name: name, ingredients: ingredients));
                },
                child: Text('Save Cocktail'),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                  padding: EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                ),
              )
            else
              Text(
                'Total must be exactly 100ml to save',
                style: TextStyle(color: Colors.red, fontSize: 16),
              )
          ],
        ),
      ),
    );
  }
}