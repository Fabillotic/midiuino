/* midiuino - MIDI library for Arduino
 *
 * MIT License
 *
 * Copyright (C) 2025 Fabillotic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include "midiuino.h"

MIDIReceiver::MIDIReceiver() {
  _stream = NULL;
  _status = 0;
  _data_buf = -1;
}

void MIDIReceiver::begin(Stream &stream) {
  _stream = &stream;
}

static int required_data_bytes(int status) {
  if((status & 0xF0) == 0x80) { // Note off
    return 2;
  }
  else if((status & 0xF0) == 0x90) { // Note on
    return 2;
  }
  else if((status & 0xF0) == 0xA0) { // Note Aftertouch
    return 2;
  }
  else if((status & 0xF0) == 0xB0) { // Control Change
    return 2;
  }
  else if((status & 0xF0) == 0xC0) { // Program Change
    return 1;
  }
  else if((status & 0xF0) == 0xD0) { // Channel Aftertouch
    return 1;
  }
  else if((status & 0xF0) == 0xE0) { // Pitch Bend
    return 2;
  }
  return -1;
}

static int has_running_status(int status) {
  return status >= 0x80 && status <= 0xEF;
}

int midi_message_from_bytes(int status, int data1, int data2, MIDIMessage *msg) {
  if((status & 0xF0) == 0x80) { // Note off
    if(data1 < 0 || data2 < 0) return 0;
    msg->type = MIDI_MESSAGE_NOTE_OFF;
    msg->data.note_off.channel = (status & 0xF) + 1;
    msg->data.note_off.note = data1;
    msg->data.note_off.velocity = data2;
    return 1;
  }
  else if((status & 0xF0) == 0x90) { // Note on
    if(data1 < 0 || data2 < 0) return 0;
    msg->type = MIDI_MESSAGE_NOTE_ON;
    msg->data.note_on.channel = (status & 0xF) + 1;
    msg->data.note_on.note = data1;
    msg->data.note_on.velocity = data2;
    return 1;
  }
  else if((status & 0xF0) == 0xA0) { // Note Aftertouch
    if(data1 < 0 || data2 < 0) return 0;
    msg->type = MIDI_MESSAGE_NOTE_AFTERTOUCH;
    msg->data.note_aftertouch.channel = (status & 0xF) + 1;
    msg->data.note_aftertouch.note = data1;
    msg->data.note_aftertouch.velocity = data2;
    return 1;
  }
  else if((status & 0xF0) == 0xB0) { // Control Change
    if(data1 < 0 || data2 < 0) return 0;
    msg->type = MIDI_MESSAGE_CONTROL_CHANGE;
    msg->data.control_change.channel = (status & 0xF) + 1;
    msg->data.control_change.control = data1;
    msg->data.control_change.value = data2;
    return 1;
  }
  else if((status & 0xF0) == 0xC0) { // Program Change
    if(data1 < 0) return 0;
    msg->type = MIDI_MESSAGE_PROGRAM_CHANGE;
    msg->data.program_change.channel = (status & 0xF) + 1;
    msg->data.program_change.program = data1;
    return 1;
  }
  else if((status & 0xF0) == 0xD0) { // Channel Aftertouch
    if(data1 < 0) return 0;
    msg->type = MIDI_MESSAGE_CHANNEL_AFTERTOUCH;
    msg->data.channel_aftertouch.channel = (status & 0xF) + 1;
    msg->data.channel_aftertouch.velocity = data1;
    return 1;
  }
  else if((status & 0xF0) == 0xE0) { // Pitch Bend
    if(data1 < 0 || data2 < 0) return 0;
    msg->type = MIDI_MESSAGE_PITCH_BEND;
    msg->data.pitch_bend.channel = (status & 0xF) + 1;
    msg->data.pitch_bend.value = (data2 << 7) | data1;
    return 1;
  }
  else { // Unrecognized status byte
    return 0;
  }
}

int MIDIReceiver::recv(MIDIMessage *msg) {
  int byte, data1, data2, req_bytes;

  if(!_stream) goto invalid_msg;

  byte = _stream->read();
  if(byte < 0) goto incomplete_msg; // No available bytes
  if(byte >= 0xF8) goto incomplete_msg; // System Real Time message

  data1 = -1;
  data2 = -1;

  if((byte & 0x80)) { // Status byte
    _status = byte;
    _data_buf = -1;
  }
  else if(_data_buf < 0) { // No buffered byte, byte is first data byte
    _data_buf = byte;
  }
  else { // Second data byte
    data2 = byte;
  }

  // _data_buf now contains first data byte, if any
  data1 = _data_buf;

  // No active status
  if(!_status) goto invalid_msg;

  // System Exlusive (SysEx) messages
  if(_status == 0xF0) {
    while((byte = _stream->peek()) >= 0) {
      // SysEx is counted as invalid right now
      if(byte == 0xF7) {
        _stream->read();
        goto invalid_msg;
      }
      if((byte & 0x80)) goto invalid_msg;
      _stream->read();
    }
    goto incomplete_msg;
  }

  // End of Exclusive (EOX)
  if(_status == 0xF7) goto invalid_msg; // SysEx is counted as invalid right now

  req_bytes = required_data_bytes(_status);
  if(req_bytes >= 1 && data1 < 0) {
    byte = _stream->peek();
    if(byte < 0) goto incomplete_msg; // No available bytes
    if(byte > 0xF8) goto incomplete_msg; // System Real Time message
    if((byte & 0x80)) goto invalid_msg; // Interrupted by new status
    _stream->read();
    _data_buf = byte;
    data1 = byte;
  }
  if(req_bytes >= 2 && data2 < 0) {
    byte = _stream->peek();
    if(byte < 0) goto incomplete_msg; // No available bytes
    if(byte > 0xF8) goto incomplete_msg; // System Real Time message
    if((byte & 0x80)) goto invalid_msg; // Interrupted by new status
    _stream->read();
    data2 = byte;
  }

  if(!midi_message_from_bytes(_status, data1, data2, msg)) {
    goto invalid_msg;
  }

  _data_buf = -1;
  if(!has_running_status(_status)) _status = -1;

  return 1;

invalid_msg:
  _status = 0;
  _data_buf = -1;

incomplete_msg:
  return 0;
}
