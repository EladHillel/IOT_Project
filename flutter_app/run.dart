import 'dart:convert';
import 'dart:html' as html; // For web download
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

void main() => runApp(CocktailApp());

class CocktailApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: CocktailListScreen(),
    );
  }
}

class Cocktail {
  String name;
  Map<String, int> ingredients;

  Cocktail({required this.name, required this.ingredients});

  bool get isComplete => ingredients.values.reduce((a, b) => a + b) == 100;

  Map<String, dynamic> toJson() {
    return {
      'name': name,
      'ingredients': ingredients,
    };
  }
}

class CocktailListScreen extends StatefulWidget {
  @override
  _CocktailListScreenState createState() => _CocktailListScreenState();
}

class _CocktailListScreenState extends State<CocktailListScreen> {
  List<Cocktail> cocktails = List.generate(
      9,
      (index) => Cocktail(name: 'Cocktail ${index + 1}', ingredients: {
            'Drink 1': 0,
            'Drink 2': 0,
            'Drink 3': 0,
            'Drink 4': 0,
          }));

  // BLE state
  List<BluetoothDevice> devicesList = [];
  BluetoothDevice? connectedDevice;
  BluetoothCharacteristic? writeCharacteristic;

  // Start scanning for BLE devices
  void scanForDevices() {
    devicesList.clear();
    FlutterBluePlus.startScan(timeout: const Duration(seconds: 5));
    FlutterBluePlus.scanResults.listen((results) {
      for (ScanResult result in results) {
        if (!devicesList.contains(result.device)) {
          setState(() {
            devicesList.add(result.device);
          });
        }
      }
    });
  }

  // Connect to a selected BLE device
  Future<void> connectToDevice(BluetoothDevice device) async {
    await device.connect();
    setState(() {
      connectedDevice = device;
    });

    List<BluetoothService> services = await device.discoverServices();
    for (var service in services) {
      for (var characteristic in service.characteristics) {
        if (characteristic.properties.write) {
          writeCharacteristic = characteristic;
          break;
        }
      }
    }
  }

  // Send cocktail JSON over BLE
  void sendCocktailsOverBLE(List<Cocktail> cocktails) async {
    if (writeCharacteristic == null) {
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('No writable characteristic found!')));
      return;
    }
    final jsonString =
        jsonEncode(cocktails.map((c) => c.toJson()).toList());
    List<int> data = utf8.encode(jsonString);

    try {
      await writeCharacteristic!.write(data, withoutResponse: false);
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Cocktail menu sent over BLE!')));
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Failed to send data: $e')));
    }
  }

  // Save JSON file locally (web)
  void saveCocktailsJson(List<Cocktail> cocktails) {
    final jsonString = jsonEncode(cocktails.map((c) => c.toJson()).toList());
    final blob = html.Blob([jsonString], 'application/json');
    final url = html.Url.createObjectUrlFromBlob(blob);
    final anchor = html.AnchorElement(href: url)
      ..setAttribute("download", "cocktails.json")
      ..click();
    html.Url.revokeObjectUrl(url);
    ScaffoldMessenger.of(context)
        .showSnackBar(SnackBar(content: Text('Cocktails JSON saved.')));

    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: Text('Cocktail JSON Preview'),
        content: SingleChildScrollView(
          child: Text(jsonString),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: Text('Close'),
          )
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    int completeCount = cocktails.where((c) => c.isComplete).length;

    return Scaffold(
      appBar: AppBar(title: Text('Cocktail Menu')),
      body: Column(
        children: [
          // BLE device connection status
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: connectedDevice == null
                ? ElevatedButton(
                    onPressed: scanForDevices,
                    child: Text('Scan BLE Devices'),
                  )
                : Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text(
                          'Connected to: ${connectedDevice!.name.isNotEmpty ? connectedDevice!.name : connectedDevice!.id}'),
                      TextButton(
                        child: Text('Disconnect'),
                        onPressed: () async {
                          await connectedDevice!.disconnect();
                          setState(() {
                            connectedDevice = null;
                            writeCharacteristic = null;
                          });
                        },
                      )
                    ],
                  ),
          ),
          // List discovered BLE devices (if not connected)
          if (connectedDevice == null)
            Expanded(
              child: ListView.builder(
                itemCount: devicesList.length,
                itemBuilder: (context, index) {
                  return ListTile(
                    title: Text(devicesList[index].name.isNotEmpty
                        ? devicesList[index].name
                        : 'Unknown device'),
                    subtitle: Text(devicesList[index].id.toString()),
                    onTap: () => connectToDevice(devicesList[index]),
                  );
                },
              ),
            ),

          // Cocktail list + editing
          if (connectedDevice != null)
            Expanded(
              child: ListView.builder(
                itemCount: cocktails.length,
                itemBuilder: (context, i) {
                  return ListTile(
                    title: Text(
                        '${cocktails[i].isComplete ? '✅' : '❌'} - ${cocktails[i].name}'),
                    trailing: IconButton(
                      icon: Icon(Icons.edit),
                      onPressed: () async {
                        final updated = await Navigator.push(
                          context,
                          MaterialPageRoute(
                              builder: (context) =>
                                  EditCocktailScreen(cocktail: cocktails[i])),
                        );
                        if (updated != null) {
                          setState(() {
                            cocktails[i] = updated;
                          });
                        }
                      },
                    ),
                  );
                },
              ),
            ),

          // Send and Save buttons (only if connected and all complete)
          if (connectedDevice != null)
            Padding(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                children: [
                  if (completeCount == cocktails.length)
                    ElevatedButton(
                      onPressed: () => sendCocktailsOverBLE(cocktails),
                      child: Text('Send over BLE'),
                    ),
                  SizedBox(height: 10),
                  ElevatedButton(
                    onPressed: () => saveCocktailsJson(cocktails),
                    child: Text('Save as JSON file'),
                  ),
                ],
              ),
            ),
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
            Text('Total: $total ml'),
            for (String drink in ingredients.keys)
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text('$drink: ${ingredients[drink]} ml'),
                  Row(
                    children: [
                      IconButton(
                        icon: Icon(Icons.remove),
                        onPressed: () => updateIngredient(drink, -10),
                      ),
                      IconButton(
                        icon: Icon(Icons.add),
                        onPressed: () => updateIngredient(drink, 10),
                      ),
                    ],
                  )
                ],
              ),
            Spacer(),
            if (total == 100)
              ElevatedButton(
                onPressed: () {
                  Navigator.pop(context,
                      Cocktail(name: name, ingredients: ingredients));
                },
                child: Text('Save'),
              ),
          ],
        ),
      ),
    );
  }
}
