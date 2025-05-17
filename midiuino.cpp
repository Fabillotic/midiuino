#include <Arduino.h>
#include "midiuino.h"

MIDIReceiver::MIDIReceiver() {
  _stream = NULL;
  _status = 0;
}

void MIDIReceiver::begin(Stream &stream) {
  _stream = &stream;
}

/* Receive MIDI message
 * Returns 1 on success, otherwise 0.
 */
int MIDIReceiver::recv(MIDIMessage *msg) {
  if(!_stream) return 0;

  int byte = _stream->read(); // Read new byte
  if(byte < 0) return 0;

  // System Real Time messages
  if(byte >= 0xF8 && byte <= 0xFE) return 0;
  if(byte == 0xFF) return 0; // System Reset

  int status = _status;
  int data = -1;
  if((byte & 0x80) > 0) { // Read a new status byte
    status = byte;
    _data1 = -1;
    _data2 = -1;

    // Check for next data byte
    byte = _stream->peek();
    if(byte >= 0 && byte <= 0x80) {
      _stream->read();
      data = byte;
    }
  }
  else { // Read a new data byte
    data = byte;
  }

  if((status & 0x80) == 0) return 0; // No current status, discard byte

  // Set _data1 or _data2 if a new data byte was read
  if(data >= 0 && _data1 < 0) _data1 = data;
  else if(data >= 0 && _data2 < 0) _data2 = data;

  // Remain in status by default (in case more data bytes are required or running status)
  _status = status;

  if(status == 0xF0) { // Start of System Exclusive (SysEx) message
    _status = 0;
    return 0;
  }
  if(status == 0xF7) { // EOX (end of SysEx)
    _status = 0;
    return 0;
  }

  if((status & 0xF0) == 0x80) { // Note off
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_NOTE_OFF;
    msg->data.note_off.channel = (status & 0xF) + 1;
    msg->data.note_off.note = _data1;
    msg->data.note_off.velocity = _data2;

    _data1 = -1;
    _data2 = -1;
    return 1;
  }
  else if((status & 0xF0) == 0x90) { // Note on
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_NOTE_ON;
    msg->data.note_on.channel = (status & 0xF) + 1;
    msg->data.note_on.note = _data1;
    msg->data.note_on.velocity = _data2;

    _data1 = -1;
    _data2 = -1;
    return 1;
  }
  else if((status & 0xF0) == 0xA0) { // Note Aftertouch
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_NOTE_AFTERTOUCH;
    msg->data.note_aftertouch.channel = (status & 0xF) + 1;
    msg->data.note_aftertouch.note = _data1;
    msg->data.note_aftertouch.velocity = _data2;

    _data1 = -1;
    _data2 = -1;
    return 1;
  }
  else if((status & 0xF0) == 0xB0) { // Control Change
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_CONTROL_CHANGE;
    msg->data.control_change.channel = (status & 0xF) + 1;
    msg->data.control_change.control = _data1;
    msg->data.control_change.value = _data2;

    _data1 = -1;
    _data2 = -1;
    return 1;
  }
  else if((status & 0xF0) == 0xC0) { // Program Change
    if(_data1 < 0) return 0;

    msg->type = MIDI_MESSAGE_PROGRAM_CHANGE;
    msg->data.program_change.channel = (status & 0xF) + 1;
    msg->data.program_change.program = _data1;

    _data1 = -1;
    _data2 = -1;
    return 1;
  }
  else if((status & 0xF0) == 0xD0) { // Channel Aftertouch
    if(_data1 < 0) return 0;

    msg->type = MIDI_MESSAGE_CHANNEL_AFTERTOUCH;
    msg->data.channel_aftertouch.channel = (status & 0xF) + 1;
    msg->data.channel_aftertouch.velocity = _data1;

    _data1 = -1;
    _data2 = -1;
    return 1;
  }
  else if((status & 0xF0) == 0xE0) { // Pitch Bend
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_PITCH_BEND;
    msg->data.pitch_bend.channel = (status & 0xF) + 1;
    msg->data.pitch_bend.value = (_data2 << 7) | _data1;

    _data1 = -1;
    _data2 = -1;
    return 1;
  }
  else { // Unrecognized status byte
    _status = 0; // Reset status (i.e. ignore any following data bytes)
    _data1 = -1;
    _data2 = -1;
    return 0;
  }
}