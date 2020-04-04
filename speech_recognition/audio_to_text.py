"""Utility to convert wav files to text."""

import os
import sys
import pocketsphinx as ps
import sphinxbase


def convert_audio_to_text(audio_file):
  """converts a given audio file in wav format to text
  
  Parameters:
    audio_file (string): path of audio file to be processed
  
  Returns:
    string: decoded text from speech file
  """

  wavInput = open(audio_file, 'rb')
  speech_decoder = get_speech_decoder()
  speech_decoder.start_utt()
  while True:
    buffer = wavInput.read(1024)
    if buffer:
      speech_decoder.process_raw(buffer, False, False)
    else:
      break
  speech_decoder.end_utt()
  decode_result = [segment.word for segment in speech_decoder.seg()]

  return decode_result


def get_speech_decoder():
  """returns a model for speech recognition"""
  
  model_path = ps.get_model_path()

  config = ps.Decoder.default_config()
  config.set_string('-hmm', os.path.join(model_path, 'en-us'))
  config.set_string('-lm', os.path.join(model_path, 'en-us.lm.bin'))
  config.set_string('-dict', os.path.join(model_path, 'cmudict-en-us.dict'))
  speech_decoder = ps.Decoder(config)
  
  return speech_decoder


if __name__ == '__main__':

  audio_file = sys.argv[1]
  audio_text = convert_audio_to_text(audio_file)
  print("I HEARD...")
  print(audio_text)
