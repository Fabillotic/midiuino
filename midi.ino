void setup() {
  Serial.begin(9600);
  Serial2.begin(31250, SERIAL_8N1, 22, 23);
}

void loop() {
  int x = Serial2.read();
  if(x > 0) {
    if(x != 0xFE && x != 0xF8) {
      Serial.println(x, HEX);
    }
  }
}

struct MIDIMessageNoteOff {
  int channel;
  int note;
  int velocity;
};

struct MIDIMessageNoteOn {
  int channel;
  int note;
  int velocity;
};

struct MIDIMessageNoteAftertouch {
  int channel;
  int note;
  int velocity;
};

struct MIDIMessageControlChange {
  int channel;
  int control;
  int value;
};

struct MIDIMessageProgramChange {
  int channel;
  int program;
};

struct MIDIMessageChannelAftertouch {
  int channel;
  int note;
  int velocity;
};

struct MIDIMessagePitchBend {
  int channel;
  int value;
};

class MIDI {
  public:
  MIDI();
  void begin(Stream* stream);
  void begin(HardwareSerial& serial);
  void begin(HardwareSerial& serial, int rxPin, int txPin);
  void recv();

  private:
  Stream* _stream;
  int _run_status; // Current running status or 0 if none
  int _avail;
};

MIDI::MIDI() {
  _stream = NULL;
  _avail = 0;
}

void MIDI::begin(Stream* stream) {
  _stream = stream;
}

void MIDI::begin(HardwareSerial& serial) {
  serial.begin(31250);
  _stream = &serial;
}

void MIDI::begin(HardwareSerial& serial, int rxPin, int txPin) {
  serial.begin(31250, rxPin, txPin);
  _stream = &serial;
}

void MIDI::recv() {
  if(!_stream->available()) return;
  int status = _stream->peek(); // Read status
  if(status < 0) return;

  int status_present = 1;
  if(!(status & 0x80)) {
    status_present = 0;
    if(_run_status) status = _run_status; // Keep running status
    else return; // Ignore unexpected data byte
  }

  // NOTE: page spec 93 byte reading, running status

  int count = status_present; // Need at least one byte if status
  int num_data_bytes[] = {2, 2, 2, 2, 1, 1, 2};
  if((status >> 4) <= 0xE) count += num_data_bytes[(status >> 4) & 0x7];
  if(_stream->available() < count) return;

  int data1 = -1;
  int data2 = -1;

  if(count > 0) _stream->read(); // Finally consume status byte
  if(count > 1) {
    data1 = _stream->read();
    if(data1 < 0) return;
  }
  if(count > 2) {
    data2 = _stream->read();
    if(data2 < 0) return;
  }

  // TODO: Ignore real time status
  // TODO: Ignore SysEx
  // TODO: Set _run_status based on status here
}