#include "midiuino.h"

MIDIReceiver midi;

void setup() {
  Serial.begin(9600);
  Serial2.begin(MIDI_BAUD, SERIAL_8N1, 22, 23);
  midi.begin(Serial2);
}

void loop() {
  MIDIMessage msg;

  if(midi.recv(&msg)) {
    Serial.print("MIDI MESSAGE [");
    switch(msg.type) {
    case MIDI_MESSAGE_NOTE_OFF: Serial.print("NOTE OFF "); Serial.print(msg.data.note_off.note); break;
    case MIDI_MESSAGE_NOTE_ON: Serial.print("NOTE ON "); Serial.print(msg.data.note_on.note); break;
    case MIDI_MESSAGE_CONTROL_CHANGE: Serial.print("CONTROL "); Serial.print(msg.data.control_change.control); break;
    }
    Serial.println("]");
  }
}