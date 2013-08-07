#ifndef PLAYSOUND_H
#define PLAYSOUND_H

int createALContext();
void al_cleanup();

void playSound();
void printDeviceString(); 

int load_raw(char* buf_beg, size_t filesize);
int is_still_playing();

#endif
