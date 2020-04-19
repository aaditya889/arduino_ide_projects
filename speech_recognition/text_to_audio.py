"""Utility to convert text to audio file."""

import os
import sys
from gtts import gTTS

audio_base_path = "./audios/mp3"

def convert_text_to_audio(message, audio_filename):
  """converts a given text to audio file
  
  Parameters:
    message (string): text in english
    audio_filename (string): output audio file name
  """

  language = 'en'
  file_ext = '.mp3'
  file_path = audio_base_path + audio_filename + file_ext
  audio = gTTS(
    text=message,
    lang=language,
    slow=False
  )
  audio.save(file_path)
  os.system("mpg123 " + file_path)
  

if __name__ == '__main__':

  audio_text = sys.argv[1]
  convert_text_to_audio(audio_text, "test")
