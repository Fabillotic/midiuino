#include <midiuino.h>

MIDIReceiver midi;

void setup() {
  Serial.begin(115200);
  Serial2.begin(MIDI_BAUD, SERIAL_8N1, 22, 23);
  midi.begin(Serial2);
}

void loop() {
  MIDIMessage msg;

  if(midi.recv(&msg)) {
    Serial.print("MIDI ");
    switch(msg.type) {
    case MIDI_MESSAGE_NOTE_OFF:
      Serial.print("[");
      Serial.print(msg.data.note_off.channel);
      Serial.print("] NOTE OFF ");
      Serial.print(msg.data.note_off.note);
      Serial.print(" ");
      Serial.print(msg.data.note_off.velocity);
      break;
    case MIDI_MESSAGE_NOTE_ON:
      Serial.print("[");
      Serial.print(msg.data.note_on.channel);
      Serial.print("] NOTE ON ");
      Serial.print(msg.data.note_on.note);
      Serial.print(" ");
      Serial.print(msg.data.note_on.velocity);
      break;
    case MIDI_MESSAGE_NOTE_AFTERTOUCH:
      Serial.print("[");
      Serial.print(msg.data.note_aftertouch.channel);
      Serial.print("] NOTE AFTERTOUCH ");
      Serial.print(msg.data.note_aftertouch.note);
      Serial.print(" ");
      Serial.print(msg.data.note_aftertouch.velocity);
      break;
    case MIDI_MESSAGE_CONTROL_CHANGE:
      Serial.print("[");
      Serial.print(msg.data.control_change.channel);
      Serial.print("] CONTROL CHANGE ");
      Serial.print(msg.data.control_change.control);
      Serial.print(" ");
      Serial.print(msg.data.control_change.value);
      break;
    case MIDI_MESSAGE_PROGRAM_CHANGE:
      Serial.print("[");
      Serial.print(msg.data.program_change.channel);
      Serial.print("] PROGRAM CHANGE ");
      Serial.print(msg.data.program_change.program);
      break;
    case MIDI_MESSAGE_CHANNEL_AFTERTOUCH:
      Serial.print("[");
      Serial.print(msg.data.channel_aftertouch.channel);
      Serial.print("] CHANNEL AFTERTOUCH ");
      Serial.print(msg.data.channel_aftertouch.velocity);
      break;
    case MIDI_MESSAGE_PITCH_BEND:
      Serial.print("[");
      Serial.print(msg.data.pitch_bend.channel);
      Serial.print("] PITCH BEND ");
      Serial.print(msg.data.pitch_bend.value);
      break;
    }
    Serial.println();
  }
}
