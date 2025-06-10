import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';

class BluetoothListener extends StatefulWidget {
  final BluetoothConnection? connection;
  final bool isConnected;

  const BluetoothListener({
    Key? key,
    required this.connection,
    required this.isConnected,
  }) : super(key: key);

  @override
  State<BluetoothListener> createState() => _BluetoothListenerState();
}

class _BluetoothListenerState extends State<BluetoothListener> {
  StreamSubscription<Uint8List>? _streamSubscription;
  bool _isListening = false;
  String _buffer = '';

  @override
  void initState() {
    super.initState();
    _startListening();
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

  void _startListening() {
    if (widget.connection != null && widget.isConnected && !_isListening) {
      setState(() {
        _isListening = true;
      });

      _streamSubscription = widget.connection!.input!.listen(
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
    }
  }

  void _stopListening() {
    _streamSubscription?.cancel();
    _streamSubscription = null;
    setState(() {
      _isListening = false;
    });
  }

  void _onDataReceived(Uint8List data) {
    try {
      // Convert bytes to string
      String received = utf8.decode(data);
      _buffer += received;

      // Check for complete messages (assuming newline delimiter)
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

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: EdgeInsets.all(16),
      child: Row(
        children: [
          // Status indicator
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

  String _getStatusText() {
    if (!widget.isConnected) {
      return 'Not connected';
    } else if (_isListening) {
      return 'Listening for messages...';
    } else {
      return 'Connection established';
    }
  }
}

// Alternative version with custom popup styling
class CustomBluetoothPopup extends StatelessWidget {
  final String message;
  final VoidCallback onClose;

  const CustomBluetoothPopup({
    Key? key,
    required this.message,
    required this.onClose,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Dialog(
      backgroundColor: Colors.transparent,
      child: Container(
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(20),
          boxShadow: [
            BoxShadow(
              color: Colors.black26,
              blurRadius: 10,
              offset: Offset(0, 5),
            ),
          ],
        ),
        padding: EdgeInsets.all(20),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(
              padding: EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.blue[50],
                shape: BoxShape.circle,
              ),
              child: Icon(
                Icons.bluetooth_connected,
                color: Colors.blue,
                size: 30,
              ),
            ),
            SizedBox(height: 16),
            Text(
              'ESP32 Message',
              style: TextStyle(
                fontSize: 20,
                fontWeight: FontWeight.bold,
                color: Colors.grey[800],
              ),
            ),
            SizedBox(height: 12),
            Container(
              width: double.infinity,
              padding: EdgeInsets.all(16),
              decoration: BoxDecoration(
                color: Colors.grey[100],
                borderRadius: BorderRadius.circular(10),
              ),
              child: Text(
                message,
                style: TextStyle(
                  fontSize: 16,
                  color: Colors.black87,
                ),
                textAlign: TextAlign.center,
              ),
            ),
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: onClose,
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.blue,
                foregroundColor: Colors.white,
                padding: EdgeInsets.symmetric(horizontal: 30, vertical: 12),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(25),
                ),
              ),
              child: Text(
                'OK',
                style: TextStyle(
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}