import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

class BluetoothListener extends StatefulWidget {
  final BluetoothDevice device;
  final Guid serviceUuid;
  final Guid characteristicUuid;
  final bool isConnected;

  const BluetoothListener({
    Key? key,
    required this.device,
    required this.serviceUuid,
    required this.characteristicUuid,
    required this.isConnected,
  }) : super(key: key);

  @override
  State<BluetoothListener> createState() => _BluetoothListenerState();
}

class _BluetoothListenerState extends State<BluetoothListener> {
  StreamSubscription<List<int>>? _characteristicSubscription;
  BluetoothCharacteristic? _characteristic;
  bool _isListening = false;
  String _buffer = '';

  @override
  void initState() {
    super.initState();
    if (widget.isConnected) {
      _startListening();
    }
  }

  @override
  void didUpdateWidget(BluetoothListener oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.isConnected != oldWidget.isConnected) {
      if (widget.isConnected) {
        _startListening();
      } else {
        _stopListening();
      }
    }
  }

  @override
  void dispose() {
    _stopListening();
    super.dispose();
  }

  Future<void> _startListening() async {
    if (!_isListening && widget.isConnected) {
      setState(() {
        _isListening = true;
      });

      // Discover services and find the characteristic
      List<BluetoothService> services = await widget.device.discoverServices();
      for (BluetoothService service in services) {
        if (service.uuid == widget.serviceUuid) {
          for (BluetoothCharacteristic c in service.characteristics) {
            if (c.uuid == widget.characteristicUuid) {
              _characteristic = c;
              await c.setNotifyValue(true);
              _characteristicSubscription = c.value.listen(
                _onDataReceived,
                onError: (error) {
                  print('Bluetooth listening error: $error');
                  setState(() {
                    _isListening = false;
                  });
                },
                onDone: () {
                  print('Bluetooth connection closed');
                  setState(() {
                    _isListening = false;
                  });
                },
              );
              break;
            }
          }
        }
      }
    }
  }

  void _stopListening() {
    _characteristicSubscription?.cancel();
    _characteristicSubscription = null;
    _characteristic?.setNotifyValue(false);
    setState(() {
      _isListening = false;
    });
  }

  void _onDataReceived(List<int> data) {
    try {
      String received = utf8.decode(Uint8List.fromList(data));
      _buffer += received;

      while (_buffer.contains('\n')) {
        int newlineIndex = _buffer.indexOf('\n');
        String message = _buffer.substring(0, newlineIndex).trim();
        _buffer = _buffer.substring(newlineIndex + 1);

        if (message.isNotEmpty) {
          _showMessagePopup(message);
        }
      }
    } catch (e) {
      print('Error processing received data: $e');
    }
  }

  void _showMessagePopup(String message) {
    showDialog(
      context: context,
      barrierDismissible: true,
      builder: (BuildContext context) {
        return AlertDialog(
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(15),
          ),
          title: Row(
            children: [
              Icon(
                Icons.bluetooth,
                color: Colors.blue,
                size: 24,
              ),
              SizedBox(width: 8),
              Text(
                'Message from ESP32',
                style: TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ],
          ),
          content: Container(
            constraints: BoxConstraints(
              minHeight: 60,
              maxHeight: 200,
            ),
            child: SingleChildScrollView(
              child: Container(
                padding: EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: Colors.grey[100],
                  borderRadius: BorderRadius.circular(8),
                ),
                child: Text(
                  message,
                  style: TextStyle(
                    fontSize: 16,
                    color: Colors.black87,
                  ),
                ),
              ),
            ),
          ),
          actions: [
            TextButton(
              onPressed: () {
                Navigator.of(context).pop();
              },
              child: Text(
                'OK',
                style: TextStyle(
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                  color: Colors.blue,
                ),
              ),
            ),
          ],
        );
      },
    );
  }

  String _getStatusText() {
    if (!widget.isConnected) {
      return 'Not connected';
    } else if (_isListening) {
      return 'Listening for messages...';
    } else {
      return 'Connection established';
    }
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: EdgeInsets.all(16),
      child: Row(
        children: [
          Container(
            width: 12,
            height: 12,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: (widget.isConnected && _isListening)
                  ? Colors.green
                  : Colors.red,
            ),
          ),
          SizedBox(width: 8),
          Text(
            _getStatusText(),
            style: TextStyle(
              fontSize: 14,
              color: Colors.grey[600],
            ),
          ),
          Spacer(),
          if (widget.isConnected && _isListening)
            Icon(
              Icons.hearing,
              color: Colors.blue,
              size: 20,
            ),
        ],
      ),
    );
  }
}
