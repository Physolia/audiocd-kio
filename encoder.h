/*
  Copyright (C) 2004 Benjamin Meyer <ben + audiocd at meyerhome dot net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef AUDIOCD_ENCODER_H
#define AUDIOCD_ENCODER_H

#include <sys/types.h>
#include <kio/slavebase.h>

class KConfig;

class Encoder {

public:
  /**
   * Constructor.
   * @param slave parent that this classes can use to call data() with
   * when finished encoding bits.
   */ 
  Encoder(KIO::SlaveBase *slave) : ioslave(slave) {};
  
  /**
   * Deconstructor.
   */
  virtual ~Encoder(){};
  
  /**
   * Initiallizes the decoder, loading libraries, etc.  Encoders
   * that don't return true will will deleted and not used.
   * @returns false if unable to initialize the encoder.
   */
  virtual bool init() = 0;

  /**
   * The encoder should read in its config data here.
   * When possible using the type() for the group is best.
   * config is shared with all plugins so stay in your own
   * group.
   * @param config pointer to active KConfig object. 
   */ 
  virtual void loadSettings(KConfig *config) = 0;

  /**
   * Helper function to determine the end size of a
   * encoded file.
   * @param time_secs the lengh of the audio track in seconds.
   * @returns the size of a file if it is time_secs in length.
   */ 
  virtual unsigned long size(long time_secs) = 0;

  /**
   * @returns the generic user string type/name of this encoder
   * Examples: "MP3", "Ogg Vorbis", "Wav", "FID Level 2", etc
   */ 
  virtual QString type() = 0;
  
  /**
   * @returns the mime type for the files this encoder produces.
   * Example: "audio/x-wav"
   */ 
  virtual const char * mimeType() = 0;

  /**
   * @returns the file type for the files this encoder produces.
   * Used in naming of the file for example foo.mp3
   * Examples: "mp3", "ogg", "wav"
   */ 
  virtual const char * fileType() = 0;

  /**
   * Before the read functions are called this is 
   * called to allow the encoders to store the cddb
   * information if they want to so it can be inserted 
   * where neccessary (start, middle, end, or combos etc).
   */ 
  virtual void fillSongInfo(QString trackName,
		            QString cdArtist,
			    QString cdTitle,
			    QString cdCategory,
			    int trackNumber,
			    int cdYear) = 0;
  
  /**
   * Perform any initial file creation necessary for a new song (that
   * has just been sent via fillSongInfo())
   * @param size - the total binary size of the end file (via size()).
   * @return size of the data that was created by this function.
   */ 
  virtual long readInit(long size) = 0;
  
  /**
   * Passes a little bit of cd data to be encoded
   * This function is most likly called many many times.
   * @param buf pointer to the audio that has been read in so far
   * @param frames the number of frames of audio that are in buf
   * @return size of the data that was created by this function.
   */
  virtual long read(int16_t * buf, int frames) = 0;

  /**
   * Perform any final file creation/padding that is necessary
   * @return size of the data that was created by this function.
   */ 
  virtual long readCleanup() = 0;

protected:
  /**
   * Pointer to the ioslave that is running this encoder.
   * Used (only?) for the data() function to pass back encoded data.
   */
  KIO::SlaveBase *ioslave;

};

#endif // AUDIOCD_ENCODER_H

